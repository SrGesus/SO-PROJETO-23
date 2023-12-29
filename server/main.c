#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

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

  // Opening with Read Write causes thread to block when nothing is sent (instead of EOF)
  // Means server MUST be started before client
  *register_fifo = open(register_pipe_path, O_RDWR);
  if (*register_fifo < 0) {
    fprintf(stderr, "[ERR]: Failed to open register fifo %s: %s\n", register_pipe_path, strerror(errno));
    return 1;
  }
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

  while (1) {
    char buffer[BUFSIZ];
    char * req_pipe_path = buffer;
    char * resp_pipe_path;
    ssize_t ret = read(register_fifo, buffer, BUFSIZ - 1);
    if (ret == -1) {
      // ret == -1 indicates error
      fprintf(stderr, "[ERR]: Failed to read register fifo: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    buffer[ret] = 0;
    if (DEBUG_REGISTER) {
      printf("[DEBUG]: Register Fifo received %zdB\n%s\n", ret, buffer);
    }
    resp_pipe_path = strchr(buffer, '\n');
    *resp_pipe_path = '\0';
    resp_pipe_path++;

    if (DEBUG_REGISTER) {
      printf("[DEBUG]: Requests pipe: \"%s\", Response pipe: \"%s\"\n", req_pipe_path, resp_pipe_path);
    }

    int req_pipe = open(argv[1], O_RDONLY);
    if (req_pipe == -1) {
      fprintf(stderr, "[ERR]: Failed to open request pipe %s: %s\n", argv[1], strerror(errno));
      continue;
    }
    int resp_pipe = open(argv[1], O_WRONLY);
    if (resp_pipe == -1) {
      fprintf(stderr, "[ERR]: Failed to open response pipe %s: %s\n", argv[1], strerror(errno));
      continue;
    }
    
    //TODO: Read from pipe
    //TODO: Write new client to the producer-consumer buffer
  }

  //TODO: Close Server

  // Close pipe
  close(register_fifo);

  ems_terminate();
}