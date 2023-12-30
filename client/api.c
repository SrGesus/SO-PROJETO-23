#include "api.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "../common/io.h"
#include "../common/constants.h"

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  int server_pipe, req_pipe, resp_pipe;
  char msg[ + 40 + 40];
  int req_pipe_path_size = strlen(req_pipe_path);
  int resp_pipe_path_size = strlen(resp_pipe_path);
  msg[0] = '1';

  strcpy(msg + 1, req_pipe_path);
  memset(msg + 1 + req_pipe_path_size, '\0', 40 - req_pipe_path_size);
  strcpy(msg + 1 + 40, resp_pipe_path);
 
  memset(msg + 1 + 40 + resp_pipe_path_size, '\0', 40 - resp_pipe_path_size);
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
  if (!print_nstr(server_pipe,1+40+40,msg)){
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
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}
