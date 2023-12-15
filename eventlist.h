#ifndef EVENT_LIST_H
#define EVENT_LIST_H

#define __USE_UNIX98

#include <pthread.h>
#include <stddef.h>

struct Event {
  pthread_rwlock_t
      show_lock; /// Lock to make sure no reservations while showing
  pthread_mutex_t reservation_mutex; /// Lock for getting new reservation_id
  unsigned int id;                   /// Event id
  unsigned int reservations;         /// Number of reservations for the event.

  size_t cols; /// Number of columns.
  size_t rows; /// Number of rows.

  unsigned int
      *data; /// Array of size rows * cols with the reservations for each seat.
  pthread_rwlock_t
      *seat_locks; /// Array of size rows * cols with rwlocks for each seat.
};

struct ListNode {
  struct Event *event;
  struct ListNode *next;
};

// Linked list structure
struct EventList {
  pthread_mutex_t append_mutex;
  struct ListNode *head; // Head of the list
  struct ListNode *tail; // Tail of the list
};

/// Creates a new event list.
/// @return Newly created event list, NULL on failure
struct EventList *create_list();

/// Appends a new node to the list.
/// @param list Event list to be modified.
/// @param data Event to be stored in the new node.
/// @return 0 if the node was appended successfully, 1 otherwise.
int append_to_list(struct EventList *list, struct Event *data);

/// Removes a node from the list.
/// @param list Event list to be modified.
/// @return 0 if the node was removed successfully, 1 otherwise.
void free_list(struct EventList *list);

/// Retrieves an event in the list.
/// @param list Event list to be searched
/// @param event_id Event id.
/// @return Pointer to the event if found, NULL otherwise.
struct Event *get_event(struct EventList *list, unsigned int event_id);

#endif // EVENT_LIST_H
