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
#include "server/session.h"
#include "server/parser.h"
#include "server/operations.h"

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

  // TODO: Intialize server, create worker threads

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
  while (parse_operation(&single_session) != -1)
    ;

  // while (1) {

  //   //TODO: Read from pipe
  //   //TODO: Write new client to the producer-consumer buffer
  // }

  // TODO: Close Server

  // Close pipe
  close(register_fifo);

  ems_terminate();
}