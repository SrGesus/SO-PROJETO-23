#ifndef JOBS_H
#define JOBS_H
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "parser.h"

void outputFile(char * path, const char *  newExtension){
  char *extension =  strrchr(path,'.'); //find the last ocurrence of '.'
  if (extension) strcpy(extension,newExtension);
  else strcat(path,newExtension);
}
int write_fmt(int fd, const char *restrict fmt, ...) {
  const size_t buffer_size = 256;
  char buffer[buffer_size];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buffer, buffer_size, fmt, args);

  size_t len = strlen(buffer);
  size_t done = 0;

  while (len > 0) {
    ssize_t bytes_written = write(fd, buffer + done, len);

    if (bytes_written < 0){
      fprintf(stderr, "write error: %s\n", strerror(errno));
      return -1;
    }

    /* might not have managed to write all, len becomes what remains */
    len -= (size_t)bytes_written;
    done += (size_t)bytes_written;
  }
  return 0;
}

int read_batch(int fd_in, int fd_out) {
  unsigned int event_id, delay;
  size_t num_rows, num_columns, num_coords;
  size_t xs[MAX_RESERVATION_SIZE], ys[MAX_RESERVATION_SIZE];
  printf("%d\n", fd_out);

  switch (get_next(fd_in)) {
    case CMD_CREATE:
      if (parse_create(fd_in, &event_id, &num_rows, &num_columns) != 0) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        return -1;
      }

      if (ems_create(event_id, num_rows, num_columns)) {
        fprintf(stderr, "Failed to create event\n");
      }

      break;

    case CMD_RESERVE:
      num_coords = parse_reserve(fd_in, MAX_RESERVATION_SIZE, &event_id, xs, ys);

      if (num_coords == 0) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        return -1;
      }

      if (ems_reserve(event_id, num_coords, xs, ys)) {
        fprintf(stderr, "Failed to reserve seats\n");
      }

      break;

    case CMD_SHOW:
      if (parse_show(fd_in, &event_id) != 0) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        return -1;
      }

      if (ems_show(event_id)) {
        fprintf(stderr, "Failed to show event\n");
      }

      break;

    case CMD_LIST_EVENTS:
      if (ems_list_events()) {
        fprintf(stderr, "Failed to list events\n");
      }

      break;

    case CMD_WAIT:
      if (parse_wait(fd_in, &delay, NULL) == -1) {  // thread_id is not implemented
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        return -1;
      }

      if (delay > 0) {
        printf("Waiting...\n");
        ems_wait(delay);
      }

      break;

    case CMD_INVALID:
      fprintf(stderr, "Invalid command. See HELP for usage\n");
      break;

    case CMD_HELP:
      printf(
          "Available commands:\n"
          "  CREATE <event_id> <num_rows> <num_columns>\n"
          "  RESERVE <event_id> [(<x1>,<y1>) (<x2>,<y2>) ...]\n"
          "  SHOW <event_id>\n"
          "  LIST\n"
          "  WAIT <delay_ms> [thread_id]\n"  // thread_id is not implemented
          "  BARRIER\n"                      // Not implemented
          "  HELP\n");

      break;

    case CMD_BARRIER:  // Not implemented
    case CMD_EMPTY:
      break;

    case EOC:
      ems_terminate();
      return 1;
  }
  return 0;
}

#endif // JOBS_H