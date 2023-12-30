#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/constants.h"
#include "../common/io.h"
#include "operations.h"
#include "session.h"
#include "common/op_code.h"


/// @brief Attempts to initialize register fifo pipe
/// @param register_fifo pointer where pipe file descriptor will be written
/// @param register_pipe_path path to pipe file
/// @return 0 if sucessful, 1 otherwise
int initialize_pipe(int * register_fifo, const char * register_pipe_path) {
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
    printf("[DEBUG]: Opening pipe %s (O_RDWR)\n", register_pipe_path);
  }
  // Opening with Read Write causes thread to block when nothing is sent (instead of EOF)
  // Means server MUST be started before client
  *register_fifo = open(register_pipe_path, O_RDWR);
  if (*register_fifo < 0) {
    fprintf(stderr, "[ERR]: Failed to open register fifo %s: %s\n", register_pipe_path, strerror(errno));
    return 1;
  }
  
  return 0;
}

/// @brief Reads from register fifo into session struct
/// @param session Pointer to struct to store session data.
/// @param register_fifo pipe file descriptor
/// @return 0 if sucessful, 1 otherwise
int initiate_session(session_t * session, int register_fifo) {
  const size_t BUFFER_SIZE = 1 /* OP_CODE */ + 40*2 /* pipe_paths*/;
  char buffer[BUFFER_SIZE];
  ssize_t ret = read(register_fifo, buffer, BUFFER_SIZE);
  if (ret == -1) {
    // ret == -1 indicates error
    fprintf(stderr, "[ERR]: Failed to read register fifo: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (DEBUG_REGISTER) {
    printf("[DEBUG]: Register Fifo received %zdB\n", ret);
    fputs(buffer, stdout);
    printf("\n");
  }

  buffer[BUFFER_SIZE-1] = '\0';
  buffer[BUFFER_SIZE-1-40] = '\0';

  char * req_pipe_path = buffer+1;
  char * resp_pipe_path = buffer+1+40;

  if (DEBUG_REGISTER) {
    printf("[DEBUG]: Requests pipe: \"%s\", Response pipe: \"%s\"\n", req_pipe_path, resp_pipe_path);
  }

  if (DEBUG_IO) {
    printf("[DEBUG]: Opening pipe %s (O_RDONLY)\n", req_pipe_path);
  }

  int req_pipe = open(req_pipe_path, O_RDONLY);
  if (req_pipe == -1) {
    fprintf(stderr, "[ERR]: Failed to open request pipe %s: %s\n", req_pipe_path, strerror(errno));
    return 1;
  }

  if (DEBUG_IO) {
    printf("[DEBUG]: Opening pipe %s (O_WRONLY)\n", resp_pipe_path);
  }
  
  int resp_pipe = open(resp_pipe_path, O_WRONLY);
  if (resp_pipe == -1) {
    fprintf(stderr, "[ERR]: Failed to open response pipe %s: %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }

  session->request_pipe = req_pipe;
  session->response_pipe = resp_pipe;

  return 0;
}

int parse_create(session_t * session) {
  char next;
  unsigned int event_id, num_rows, num_cols;

  if (parse_uint(session->request_pipe, &event_id, &next) || next != SEPARATOR_CHAR) {
    cleanup(session->request_pipe);
    return 1;
  }
  if (parse_uint(session->request_pipe, &num_rows, &next) || next != SEPARATOR_CHAR) {
    cleanup(session->request_pipe);
    return 1;
  }
  if (parse_uint(session->request_pipe, &num_cols, &next)) {
    cleanup(session->request_pipe);
    return 1;
  }

  int result = ems_create(event_id, num_rows, num_cols);

  print_uint(session->response_pipe, result);
  
  return result;
}

/// @brief Parses and executes a single operation
/// @param session pointer to current session
/// @return -1 if operation was QUIT, 0 if operation sucessful, 1 otherwise;
int parse_operation(session_t * session) {
    char operation = '0';

    ssize_t read_bytes = read(session->request_pipe, &operation, 1);
    if (read_bytes == -1) {
      fprintf(stderr, "[ERR]: Failed to read request\n");
      return 1;
    } else if (read_bytes == 0) {
      // Finished reading
      return 0;
    }

    unsigned int session_id;
    char _i;
    parse_uint(session->request_pipe, &session_id, &_i);

    if (DEBUG_REQUEST)
      printf("[DEBUG]: Received operation %c in session %u\n", operation, session_id);

    switch (operation) {
    case QUIT:
      return 0;
    case CREATE:

      break;
    case RESERVE:

      break;
    case SHOW:

      break;
    case LIST:

      break;
    default:
      fprintf(stderr, "[ERR]: Invalid operation\n");
      break;;
    }
    return 0;
}

int main(int argc, char* argv[]) {
  // Check arg count
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  // Parse access delay time
  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "[ERR]: Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  // Initialize EMS
  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "[ERR]: Failed to initialize EMS\n");
    return 1;
  }

  //TODO: Intialize server, create worker threads

  // Initialize pipe
  int register_fifo;
  if (initialize_pipe(&register_fifo, argv[1])) {
    return 1;
  }

  session_t single_session;
  if (initiate_session(&single_session, register_fifo)) {
    return 1;
  }

  // Single Worker thread
  while (true) {
  }

  // while (1) {
    
  //   //TODO: Read from pipe
  //   //TODO: Write new client to the producer-consumer buffer
  // }

  //TODO: Close Server

  // Close pipe
  close(register_fifo);

  ems_terminate();
}