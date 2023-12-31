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
#include "../common/io.h"

int req_pipe, resp_pipe;
unsigned int session_id;

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  const size_t BUFFER_SIZE = 1 + 2 * MAX_PIPE_PATH_SIZE;
  char msg[BUFFER_SIZE];
  int server_pipe;
  size_t req_pipe_path_size = strlen(req_pipe_path);
  size_t resp_pipe_path_size = strlen(resp_pipe_path);
  msg[0] = '1';

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
  if (write_nstr(server_pipe, BUFFER_SIZE, msg)) {
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
  // TODO: create pipes and connect to the server
  return 0;
}

int ems_quit(void) {
  // TODO: close pipes
  return 1;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  char msg[1] = "3";
  if (write_nstr(req_pipe, 1, msg)) {
    fprintf(stderr, "[ERR]: Failed to write op_code %s: %s\n", msg, strerror(errno));
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

  // TODO: send create request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  char msg[1] = "4";
  if (write_nstr(req_pipe, 1, msg)) {
    fprintf(stderr, "[ERR]: Failed to write op_code %s: %s\n", msg, strerror(errno));
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
  for (size_t i; i < num_seats; i++) {
    if (write_size(req_pipe, xs + i)) {
      fprintf(stderr, "[ERR]: Failed to write X seat: %s\n", strerror(errno));
      return 1;
    }
  }
  for (size_t i; i < num_seats; i++) {
    if (write_size(req_pipe, ys + i)) {
      fprintf(stderr, "[ERR]: Failed to write Y seat: %s\n", strerror(errno));
      return 1;
    }
  }

  // TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_show(int out_fd, unsigned int event_id) {
  // printf("%u\n",event_id);
  char msg[1] = "5";
  // char * msg = malloc(sizeof(char)*(event_id_size+3));
  // msg[0] = '5';
  // msg[1] = '|'; // TODO no longer using separators
  // sprintf(msg + 1,"%u",event_id);

  if (write_nstr(req_pipe, 1, msg)) {
    fprintf(stderr, "[ERR]: Failed to write op_code %s: %s\n", msg, strerror(errno));
    return 1;
  }
  if (write_uint(req_pipe, event_id)) {
    fprintf(stderr, "[ERR]: Failed to write event_id: %s\n", strerror(errno));
    return 1;
  }

  // TODO: send show request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}

int ems_list_events(int out_fd) {
  unsigned int op_value = 6;
  if (write_uint(req_pipe, op_value)) {
    fprintf(stderr, "[ERR]: Failed to write LIST OP_CODE: %s\n", strerror(errno));
    return 1;
  }
  // TODO: send list request to the server (through the request pipe) and wait for the response (through the response
  // pipe)
  return 0;
}
