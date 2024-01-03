#include "api.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/constants.h"
#include "common/io.h"
#include "common/op_code.h"

int req_pipe, resp_pipe;
unsigned int session_id;

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  const size_t BUFFER_SIZE = 1 + 2 * MAX_PIPE_PATH_SIZE;
  char msg[BUFFER_SIZE];
  int server_pipe;
  msg[0] = SETUP;
  memset(msg + 1, '\0', BUFFER_SIZE - 1);
  strcpy(msg + 1, req_pipe_path);
  strcpy(msg + 1 + MAX_PIPE_PATH_SIZE, resp_pipe_path);

  if ((server_pipe = open(server_pipe_path, O_RDWR)) < 0) {
    fprintf(stderr, "[ERR]: Failed to open server pipe %s: %s\n", server_pipe_path, strerror(errno));
    return 1;
  }
  if (unlink(req_pipe_path) && errno != ENOENT) {
    fprintf(stderr, "[ERR]: Failed to unlink %s: %s\n", req_pipe_path, strerror(errno));
    return 1;
  }
  if (unlink(resp_pipe_path) && errno != ENOENT) {
    fprintf(stderr, "[ERR]: Failed to unlink %s: %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }
  if (mkfifo(req_pipe_path, 0640) < 0) {
    fprintf(stderr, "[ERR]: Failed to create req_pipe %s: %s\n", req_pipe_path, strerror(errno));
    return 1;
  }
  if (mkfifo(resp_pipe_path, 0640) < 0) {
    fprintf(stderr, "[ERR]: Failed to create resp_pipe %s: %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }
  if (write_nbytes(server_pipe, msg, BUFFER_SIZE)) {
    fprintf(stderr, "[ERR]: Failed to write msg %s: %s\n", msg, strerror(errno));
    return 1;
  }
  if ((req_pipe = open(req_pipe_path, O_WRONLY)) < 0) {
    fprintf(stderr, "[ERR]: Failed to open req_pipe %s: %s\n", req_pipe_path, strerror(errno));
    return 1;
  }
  if ((resp_pipe = open(resp_pipe_path, O_RDONLY)) < 0) {
    fprintf(stderr, "[ERR]: Failed to open resp_pipe %s: %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }
  if (read_uint(resp_pipe, &session_id)) {
    fprintf(stderr, "[ERR]: Failed to read from resp_pipe: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}

int ems_quit(void) {
  char msg = QUIT;
  if (write_nbytes(req_pipe, &msg, sizeof(char))) {
    fprintf(stderr, "[ERR]: Failed to write op_code %c: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, session_id)) {
    fprintf(stderr, "[ERR]: Failed to write session_id: %s\n", strerror(errno));
    return 1;
  }
  return 1;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  char msg = CREATE;
  if (write_nbytes(req_pipe, &msg, sizeof(char))) {
    fprintf(stderr, "[ERR]: Failed to write op_code %c: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, session_id)) {
    fprintf(stderr, "[ERR]: Failed to write session_id: %s\n", strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, event_id)) {
    fprintf(stderr, "[ERR]: Failed to write event_id: %s\n", strerror(errno));
    return 1;
  }
  if (write_size(req_pipe, num_rows)) {
    fprintf(stderr, "[ERR]: Failed to write num_rows: %s\n", strerror(errno));
    return 1;
  }
  if (write_size(req_pipe, num_cols)) {
    fprintf(stderr, "[ERR]: Failed to write num_cols: %s\n", strerror(errno));
    return 1;
  }
  int result;
  if (read_int(resp_pipe, &result)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }
  return result;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  char msg = RESERVE;
  if (write_nbytes(req_pipe, &msg, sizeof(char))) {
    fprintf(stderr, "[ERR]: Failed to write op_code %c: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, session_id)) {
    fprintf(stderr, "[ERR]: Failed to write session_id: %s\n", strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, event_id)) {
    fprintf(stderr, "[ERR]: Failed to write event_id: %s\n", strerror(errno));
    return 1;
  }
  if (write_size(req_pipe, num_seats)) {
    fprintf(stderr, "[ERR]: Failed to write num_rows: %s\n", strerror(errno));
    return 1;
  }
  if (write_nbytes(req_pipe, xs, sizeof(size_t) * num_seats)) {
    fprintf(stderr, "[ERR]: Failed to write X seat: %s\n", strerror(errno));
    return 1;
  }
  if (write_nbytes(req_pipe, ys, sizeof(size_t) * num_seats)) {
    fprintf(stderr, "[ERR]: Failed to write Y seat: %s\n", strerror(errno));
    return 1;
  }
  int result;
  if (read_int(resp_pipe, &result)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }
  return result;
}

int ems_show(int out_fd, unsigned int event_id) {
  char msg = SHOW;

  if (write_nbytes(req_pipe, &msg, sizeof(char))) {
    fprintf(stderr, "[ERR]: Failed to write op_code %c: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, session_id)) {
    fprintf(stderr, "[ERR]: Failed to write session_id: %s\n", strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, event_id)) {
    fprintf(stderr, "[ERR]: Failed to write event_id: %s\n", strerror(errno));
    return 1;
  }
  int result;
  if (read_int(resp_pipe, &result)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }
  if (result) return result;

  size_t num_rows, num_cols;
  if (read_size(resp_pipe, &num_rows) ||
      read_size(resp_pipe, &num_cols)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }

  for (size_t i = 1; i <= num_rows; i++) {
    for (size_t j = 1; j <= num_cols; j++) {
      char buffer[16];
      unsigned int seat;
      if (read_uint(resp_pipe, &seat)) {
        perror("Error reading from response pipe");
        return 1;
      }
      sprintf(buffer, "%u", seat);

      if (print_str(out_fd, buffer)) {
        perror("Error writing to file descriptor");
        return 1;
      }

      if (j < num_cols) {
        if (print_str(out_fd, " ")) {
          perror("Error writing to file descriptor");
          return 1;
        }
      }
    }

    if (print_str(out_fd, "\n")) {
      perror("Error writing to file descriptor");
      return 1;
    }
  }

  return result;
}

int ems_list_events(int out_fd) {
  char msg = LIST;
  if (write_nbytes(req_pipe, &msg, sizeof(char))) {
    fprintf(stderr, "[ERR]: Failed to write op_code %c: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, session_id)) {
    fprintf(stderr, "[ERR]: Failed to write session_id: %s\n", strerror(errno));
    return 1;
  }
  int result;
  if (read_int(resp_pipe, &result)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }
  if (result) return result;

  size_t num_events;
  if (read_size(resp_pipe, &num_events)) {
    fprintf(stderr, "[ERR]: Failed to read response: %s\n", strerror(errno));
    return 1;
  }

  for (size_t i = 0; i < num_events; i++) {
    char buff[] = "Event: ";
    if (print_str(out_fd, buff)) {
      perror("Error writing to file descriptor");
      return 1;
    }

    char buffer[16];
    unsigned int id;
    if (read_uint(resp_pipe, &id)) {
      perror("Error reading from response pipe");
      return 1;
    }
    sprintf(buffer, "%u\n", id);
    if (print_str(out_fd, buffer)) {
      perror("Error writing to file descriptor");
      return 1;
    }
  }

  return result;
}
