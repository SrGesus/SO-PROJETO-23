#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

int main(int argc, char* argv[]) {
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
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

  // Initialize pipe
  int reg_pipe = open(argv[1], O_RDONLY | O_CREAT);
  if (reg_pipe == -1) {
      fprintf(stderr, "Failed to open named_pipe: %s\n", argv[1]);
      exit(EXIT_FAILURE);
  }

  //TODO: Intialize server, create worker threads

  while (1) {
    //TODO: Read from pipe
    //TODO: Write new client to the producer-consumer buffer
  }

  //TODO: Close Server

  // Close pipe
  close(reg_pipe);

  ems_terminate();
}