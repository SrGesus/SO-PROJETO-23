#include "api.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "../common/io.h"
#include "../common/constants.h"
#include <sys/types.h>
#include <sys/stat.h>

int req_pipe, resp_pipe;

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

  if((server_pipe = open(server_pipe_path, O_RDWR)) < 0){
    fprintf(stderr, "[ERR]: Failed to open server pipe %s: %s\n",server_pipe_path,strerror(errno));
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
  if (write_nstr(server_pipe, BUFFER_SIZE,msg)){
    fprintf(stderr, "[ERR]: Failed to write msg %s: %s\n", msg, strerror(errno));
    return 1;
  }

  if ((req_pipe = open(req_pipe_path,O_WRONLY)) < 0){
    fprintf(stderr, "[ERR]: Failed to open req_pipe %s: %s\n", req_pipe_path, strerror(errno));
    return 1;
  }
  if ((resp_pipe = open(resp_pipe_path,O_RDONLY)) < 0){
    fprintf(stderr, "[ERR]: Failed to open resp_pipe %s: %s\n", resp_pipe_path, strerror(errno));
    return 1;
  }
  //TODO: create pipes and connect to the server
  return 0;
}

int ems_quit(void) { 
  //TODO: close pipes
  return 1;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  int event_id_size = snprintf(NULL,0,"%u",event_id);
  char * msg = malloc(sizeof(char)*(event_id_size+3));
  msg[0] = '5';
  msg[1] = '|'; // TODO no longer using separators
  sprintf(msg + 2,"%u",event_id);
  printf("%s\n",msg);

  if (print_nstr(req_pipe,event_id_size + 3,msg)){
    fprintf(stderr, "[ERR]: Failed to write msg %s: %s\n", msg, strerror(errno));
    return 1;
  }

  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 0;
}

int ems_list_events(int out_fd) {
  unsigned int op_value = 6;
  if (print_uint(req_pipe,op_value)){
      fprintf(stderr, "[ERR]: Failed to write LIST OP_CODE: %s\n", strerror(errno));
      return 1;
  }
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 0;
}
