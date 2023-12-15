#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"
#include "reader.h"
#include "write.h"
#include "thread_manager.h"

void outputFile(char *path, const char *newExtension) {
  char *extension = strrchr(path, '.'); // find the last ocurrence of '.'
  if (extension)
    strcpy(extension, newExtension);
  else
    strcat(path, newExtension);
}

void *thread_routine(void *arg) {
  intptr_t thread_id = (intptr_t)arg;
  intptr_t result = 0;
  while (result != 1){
    result = read_line(thread_id, thread_manager->fd_in, thread_manager->fd_out);
    if (result == -1) {
      manager_parse_unlock();
      fprintf(stderr, "Invalid command. See HELP for usage\n");
    }
  }
  
  return (void *)result;
}

intptr_t read_line(intptr_t thread_id, int fd_in, int fd_out) {
  unsigned int event_id, delay;
  size_t num_rows, num_columns, num_seats;
  seat_t seats[MAX_RESERVATION_SIZE];
  //printf("%lu\n", thread_id);

  manager_parse_lock(thread_id);
  
  switch (get_next(fd_in)) {
  case CMD_CREATE:
    if (parse_create(fd_in, &event_id, &num_rows, &num_columns) != 0)
      return -1;

    manager_parse_unlock();

    if (ems_create(event_id, num_rows, num_columns)) {
      fprintf(stderr, "Failed to create event\n");
    }

    break;

  case CMD_RESERVE:
    num_seats = parse_reserve(fd_in, MAX_RESERVATION_SIZE, &event_id, seats);

    if (num_seats == 0)
      return -1;

    manager_parse_unlock();

    seat_sort(seats, num_seats);
    if (ems_reserve(event_id, num_seats, seats)) {
      fprintf(stderr, "Failed to reserve seats\n");
    }

    break;

  case CMD_SHOW:
    if (parse_show(fd_in, &event_id) != 0) {
      return -1;
    }

    manager_parse_unlock();

    if (ems_show(fd_out, event_id)) {
      fprintf(stderr, "Failed to show event\n");
    }

    break;

  case CMD_LIST_EVENTS:

    manager_parse_unlock();
    
    if (ems_list_events(fd_out)) {
      fprintf(stderr, "Failed to list events\n");
    }

    break;

  case CMD_WAIT:
    if (parse_wait(fd_in, &delay, &event_id) == -1) { // thread_id is not implemented
      return -1;
    }

    if (delay > 0) {      
      set_wait(delay, event_id);
    }

    // Stop other threads from reading 
    // until their wait time is written
    manager_parse_unlock();

    break;

  case CMD_INVALID:
    return -1;

  case CMD_HELP:
    manager_parse_unlock();
    printf(
              "Available commands:\n"
              "  CREATE <event_id> <num_rows> <num_columns>\n"
              "  RESERVE <event_id> [(<x1>,<y1>) (<x2>,<y2>) ...]\n"
              "  SHOW <event_id>\n"
              "  LIST\n"
              "  WAIT <delay_ms> [thread_id]\n" // thread_id is not implemented
              "  BARRIER\n"                     // Not implemented
              "  HELP\n");

    break;

  case CMD_BARRIER:
    thread_manager->barred = 1;
    // Stop other threads from reading before flag is changed
    manager_parse_unlock();
    break;

  case CMD_EMPTY:
    manager_parse_unlock();
    break;

  case EOC:
    manager_parse_unlock();
    return 1;
  }
  return 0;
}
