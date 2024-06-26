#include "server/parser.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/constants.h"
#include "common/io.h"
#include "common/op_code.h"
#include "operations.h"

static void cleanup(int fd) {
  char ch;
  while (read(fd, &ch, 1) == 1 && ch != '\n')
    ;
}

int initialize_pipe(int* register_fifo, const char* register_pipe_path) {
  // Remove pipe if it exists already
  if (unlink(register_pipe_path) && errno != ENOENT) {
    fprintf(stderr, "[ERR]: Failed to unlink %s: %s\n", register_pipe_path, strerror(errno));
    return 1;
  }

  // Create pipe
  if (mkfifo(register_pipe_path, 0640)) {
    fprintf(stderr, "[ERR]: Failed to create register fifo %s: %s\n", register_pipe_path, strerror(errno));
    return 1;
  }

  if (DEBUG_IO) {
    printf("[DEBUG]: Opening pipe \"%s\" (O_RDWR)\n", register_pipe_path);
  }
  // Opening with Read Write causes thread to block when nothing is sent (instead of EOF)
  *register_fifo = open(register_pipe_path, O_RDWR);
  if (*register_fifo < 0) {
    fprintf(stderr, "[ERR]: Failed to open register fifo \"%s\": %s\n", register_pipe_path, strerror(errno));
    return 1;
  }

  return 0;
}

int initiate_session(session_t* session, int register_fifo) {
  // printf("hello\n");
  const size_t BUFFER_SIZE = 1 /* OP_CODE */ + 40 * 2 /* pipe_paths*/;
  char buffer[BUFFER_SIZE];
  ssize_t ret = read(register_fifo, buffer, BUFFER_SIZE);
  if (ret == -1) {
    // ret == -1 indicates error
    if (errno == EINTR) {
      if (DEBUG) printf("[DEBUG] EINTR activated");
      return 1;
    }
    fprintf(stderr, "[ERR]: Failed to read register fifo: %s\n", strerror(errno));
    return 1;
  }

  if (DEBUG_REGISTER) {
    printf("[DEBUG]: Register Fifo received %zdB\n", ret);
    for (int i = 0; i < (int)ret; ++i) {
      printf("%c", buffer[i]);
    }
    printf("\n");
  }

  buffer[BUFFER_SIZE - 1] = '\0';
  buffer[BUFFER_SIZE - 1 - 40] = '\0';

  char* req_pipe_path = buffer + 1;
  char* resp_pipe_path = buffer + 1 + 40;

  if (DEBUG_REGISTER) {
    printf("[DEBUG]: Requests pipe: \"%s\", Response pipe: \"%s\"\n", req_pipe_path, resp_pipe_path);
  }

  if (DEBUG_IO) {
    printf("[DEBUG]: Opening pipe \"%s\" (O_RDONLY)\n", req_pipe_path);
  }

  int req_pipe = open(req_pipe_path, O_RDONLY);
  if (req_pipe == -1) {
    fprintf(stderr, "[ERR]: Failed to open request pipe \"%s\": %s\n", req_pipe_path, strerror(errno));
    return 1;
  }

  if (DEBUG_IO) {
    printf("[DEBUG]: Opening pipe \"%s\" (O_RDWR)\n", resp_pipe_path);
  }

  int resp_pipe = open(resp_pipe_path, O_WRONLY);
  if (resp_pipe == -1) {
    fprintf(stderr, "[ERR]: Failed to open response pipe \"%s\": %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }

  session->request_pipe = req_pipe;
  session->response_pipe = resp_pipe;

  return 0;
}

int parse_create(session_t* session) {
  unsigned int event_id;
  size_t num_rows, num_cols;

  if (read_uint(session->request_pipe, &event_id) || read_size(session->request_pipe, &num_rows) ||
      read_size(session->request_pipe, &num_cols)) {
    fprintf(stderr, "[ERR]: Failed to parse create operation\n");
    cleanup(session->request_pipe);
    return 1;
  }

  int result = ems_create(event_id, num_rows, num_cols);

  if (write_int(session->response_pipe, result)) {
    printf("Failed writing\n");
    return 1;
  }

  return result;
}

int parse_reserve(session_t* session) {
  unsigned int event_id;
  size_t num_seats;
  size_t Xs[MAX_RESERVATION_SIZE], Ys[MAX_RESERVATION_SIZE];

  if (read_uint(session->request_pipe, &event_id) || read_size(session->request_pipe, &num_seats)) {
    fprintf(stderr, "[ERR]: Failed to parse create operation\n");
    cleanup(session->request_pipe);
    return 1;
  }

  for (size_t i = 0; i < num_seats; i++) {
    if (read_size(session->request_pipe, Xs + i)) {
      fprintf(stderr, "[ERR]: Failed to parse create operation\n");
      cleanup(session->request_pipe);
      return 1;
    }
  }

  for (size_t i = 0; i < num_seats; i++) {
    if (read_size(session->request_pipe, Ys + i)) {
      fprintf(stderr, "[ERR]: Failed to parse create operation\n");
      cleanup(session->request_pipe);
      return 1;
    }
  }

  int result = ems_reserve(event_id, num_seats, Xs, Ys);

  write_uint(session->response_pipe, (unsigned int)result);

  return 0;
}

int parse_show(session_t* session) {
  unsigned int event_id;

  if (read_uint(session->request_pipe, &event_id)) {
    fprintf(stderr, "[ERR]: Failed to parse create operation\n");
    cleanup(session->request_pipe);
    return 1;
  }

  return ems_show(session->response_pipe, event_id);
}

int parse_operation(session_t* session) {
  char operation = '0';

  ssize_t read_bytes = read(session->request_pipe, &operation, 1);
  if (read_bytes == -1) {
    fprintf(stderr, "[ERR]: Failed to read request\n");
    return 1;
  } else if (read_bytes == 0) {
    // Finished reading
    return 1;
  }

  unsigned int session_id;
  read_uint(session->request_pipe, &session_id);

  if (DEBUG_REQUEST) printf("[DEBUG]: Received operation %c in session %u\n", operation, session->id);

  switch (operation) {
    case QUIT:
      if (DEBUG) printf("[DEBUG]: Quit session n.%d\n", session->id);
      close(session->request_pipe);
      close(session->response_pipe);
      return -1;
    case CREATE:
      return parse_create(session);
    case RESERVE:
      return parse_reserve(session);
    case SHOW:
      return parse_show(session);
    case LIST:
      return ems_list_events(session->response_pipe);
    default:
      fprintf(stderr, "[ERR]: Invalid operation\n");
      break;
  }
  return 0;
}
