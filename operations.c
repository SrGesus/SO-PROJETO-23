#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "eventlist.h"
#include "operations.h"
#include "thread_manager.h"
#include "write.h"

// meter lock antes de write_fmt
static struct EventList *event_list = NULL;
static unsigned int state_access_delay_ms = 0;

/// Calculates a timespec from a delay in milliseconds.
/// @param delay_ms Delay in milliseconds.
/// @return Timespec with the given delay.
static struct timespec delay_to_timespec(unsigned int delay_ms) {
  return (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
}

/// Gets the event with the given ID from the state.
/// @note Will wait to simulate a real system accessing a costly memory
/// resource.
/// @param event_id The ID of the event to get.
/// @return Pointer to the event if found, NULL otherwise.
static struct Event *get_event_with_delay(unsigned int event_id) {
  struct timespec delay = delay_to_timespec(state_access_delay_ms);
  nanosleep(&delay, NULL); // Should not be removed

  return get_event(event_list, event_id);
}

/// Gets the seat with the given index from the state.
/// @note Will wait to simulate a real system accessing a costly memory
/// resource.
/// @param event Event to get the seat from.
/// @param index Index of the seat to get.
/// @return Pointer to the seat.
static unsigned int *get_seat_with_delay(struct Event *event, size_t index) {
  struct timespec delay = delay_to_timespec(state_access_delay_ms);
  nanosleep(&delay, NULL); // Should not be removed

  return &event->data[index];
}

/// Gets the index of a seat.
/// @note This function assumes that the seat exists.
/// @param event Event to get the seat index from.
/// @param row Row of the seat.
/// @param col Column of the seat.
/// @return Index of the seat.
static size_t seat_index(struct Event *event, size_t row, size_t col) {
  return (row - 1) * event->cols + col - 1;
}

int ems_init(unsigned int delay_ms) {
  if (event_list != NULL) {
    fprintf(stderr, "EMS state has already been initialized\n");
    return 1;
  }

  event_list = create_list();
  state_access_delay_ms = delay_ms;

  return event_list == NULL;
}

int ems_terminate() {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  free_list(event_list);
  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  if (get_event_with_delay(event_id) != NULL) {
    fprintf(stderr, "Event already exists\n");
    return 1;
  }

  struct Event *event = malloc(sizeof(struct Event));

  if (event == NULL) {
    fprintf(stderr, "Error allocating memory for event\n");
    return 1;
  }

  event->id = event_id;
  event->rows = num_rows;
  event->cols = num_cols;
  event->reservations = 0;
  event->data = malloc(num_rows * num_cols * sizeof(unsigned int));

  if (event->data == NULL) {
    fprintf(stderr, "Error allocating memory for event data\n");
    free(event);
    return 1;
  }

  event->seat_locks = malloc(num_rows * num_cols * sizeof(pthread_mutex_t));

  if (event->seat_locks == NULL) {
    fprintf(stderr, "Error allocating memory for event seat_locks\n");
    free(event->data);
    free(event);
    return 1;
  }

  pthread_rwlock_init(&event->show_lock, NULL);
  pthread_mutex_init(&event->reservation_mutex, NULL);

  for (size_t i = 0; i < num_rows * num_cols; i++) {
    event->data[i] = 0;
    pthread_mutex_init(&(event->seat_locks[i]), NULL);
  }

  if (append_to_list(event_list, event) != 0) {
    fprintf(stderr, "Error appending event to list\n");
    free(event->data);
    free(event);
    return 1;
  }

  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, seat_t *seats) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  struct Event *event = get_event_with_delay(event_id);

  if (event == NULL) {
    fprintf(stderr, "Event not found\n");
    return 1;
  }

  size_t i;
  seat_t last_seat = (seat_t){0, 0};
  for (i = 0; i < num_seats; i++) {
    size_t row = seats[i].x;
    size_t col = seats[i].y;

    if (row <= 0 || row > event->rows || col <= 0 || col > event->cols ||
      (row == last_seat.x && col == last_seat.y)) {
      fprintf(stderr, "Invalid seat\n");
      break;
    }

    if (SHOW_LOCKS)
      printf("DEBUG: Locking seat: X: %lu, Y: %lu\n", row, col);

    // Lock requested seats
    pthread_mutex_lock(&event->seat_locks[seat_index(event, row, col)]);

    if (SHOW_LOCKS)
      printf("DEBUG: Locked seat: X: %lu, Y: %lu\n", row, col);

    if (*get_seat_with_delay(event, seat_index(event, row, col)) != 0) {
      fprintf(stderr, "Seat already reserved\n");
      i++;
      break;
    }

    last_seat = seats[i];
  }

  // If the reservation was not successful, unlock the seats.
  if (i < num_seats) {
    
    for (size_t j = 0; j < i; j++) {
      pthread_mutex_unlock(
          &event->seat_locks[seat_index(event, seats[j].x, seats[j].y)]);
    }
    return 1;
  }

  if (SHOW_LOCKS)
    printf("DEBUG: Locking reservation\n");
  pthread_mutex_lock(&event->reservation_mutex);

  if (SHOW_LOCKS)
    printf("DEBUG: Locked reservation\n");
  pthread_mutex_unlock(&event->reservation_mutex);
  unsigned int reservation_id = ++event->reservations;

  // No showing
  pthread_rwlock_rdlock(&event->show_lock);

  // Reserve seats
  for (i = 0; i < num_seats; i++) {
    size_t row = seats[i].x;
    size_t col = seats[i].y;

    // Reserve seat
    *get_seat_with_delay(event, seat_index(event, row, col)) = reservation_id;

    // Unlock Seat
    pthread_mutex_unlock(
        &event->seat_locks[seat_index(event, seats[i].x, seats[i].y)]);
  }

  pthread_rwlock_unlock(&event->show_lock);

  return 0;
}

int ems_show(int fd_out, unsigned int event_id) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  struct Event *event = get_event_with_delay(event_id);

  if (event == NULL) {
    fprintf(stderr, "Event not found\n");
    return 1;
  }

  unsigned int *buf_seat =
      malloc(sizeof(unsigned int) * (event->cols * event->rows));

  if (buf_seat == NULL) {
    fprintf(stderr, "Out of Memory\n");
    return 1;
  }

  // Write to buffer
  pthread_rwlock_wrlock(&event->show_lock);
  for (size_t i = 1; i <= event->rows; i++) {
    for (size_t j = 1; j <= event->cols; j++) {
      unsigned int *seat = get_seat_with_delay(event, seat_index(event, i, j));
      buf_seat[seat_index(event, i, j)] = *seat;
    }
  }
  pthread_rwlock_unlock(&event->show_lock);

  // Write to file
  pthread_mutex_lock(&thread_manager->print_mutex);
  for (size_t i = 1; i <= event->rows; i++) {
    for (size_t j = 1; j <= event->cols; j++) {
      write_fmt(fd_out, "%u", buf_seat[seat_index(event, i, j)]);
      if (j < event->cols) {
        write_fmt(fd_out, " ");
      }
    }
    write_fmt(fd_out, "\n");
  }
  write_fmt(fd_out, "\n");
  pthread_mutex_unlock(&thread_manager->print_mutex);

  free(buf_seat);
  return 0;
}

int ems_list_events(int fd_out) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  // Write to file
  pthread_mutex_lock(&thread_manager->print_mutex);

  if (event_list->head == NULL) {
    write_fmt(fd_out, "No events\n");
    pthread_mutex_unlock(&thread_manager->print_mutex);
    return 0;
  }

  struct ListNode *current = event_list->head;
  while (current != NULL) {
    write_fmt(fd_out, "Event: ");
    write_fmt(fd_out, "%u\n", (current->event)->id);
    current = current->next;
  }
  pthread_mutex_unlock(&thread_manager->print_mutex);

  return 0;
}

void ems_wait(unsigned int delay_ms) {
  struct timespec delay = delay_to_timespec(delay_ms);
  nanosleep(&delay, NULL);
}
