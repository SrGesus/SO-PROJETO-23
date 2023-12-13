#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sys/stat.h>
#include <sys/wait.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"
#include "reader.h"
#include "write.h"

int is_jobs_file(const char *filename) {
  if ((filename = strrchr(filename, '.')))
    return !strcmp(filename, ".jobs");
  else
    return 0;
}

int process(const char *folder, const char *file) {
  int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
  mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  char total_path[BUFSIZ];

  // total_path = jobs/job.jobs
  strcpy(total_path, folder);
  strcat(total_path, "/");
  strcat(total_path, file);
  printf("Initiating job: %s\n", total_path);
  int fd_in = open(total_path, O_RDONLY);
  if (fd_in < 0) {
    fprintf(stderr, "Failed to open file: %s", total_path);
    return -1;
  }
  outputFile(total_path, ".out"); // total_path = jobs/job.out
  int fdout = open(total_path, openFlags, filePerms);
  if (fdout < 0) {
    fprintf(stderr, "Failed to open file: %s", total_path);
    return -1;
  }
  while (read_batch(fd_in, fdout) != 1)
    ; // read_bach returns 1 when it reaches EOC
  close(fd_in);
  fsync(fdout);
  close(fdout);
  printf("Finished job: %s\n", total_path);
  return 0;
}

int main(int argc, char *argv[]) {
  // neste momento leva um delay em ms mas agora leva o dirPath tbm
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;
  int max_proc, status;
  pid_t pid; // init_pid = getpid();

  // If no max_proc
  max_proc = argc > 2 ? atoi(argv[2]) : 1;

  // state access delay is argument 3
  if (argc > 3) {
    char *endptr;
    unsigned long int delay = strtoul(argv[3], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_ms = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_ms)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

  if (argc > 1) {
    int active_proc = 0;
    DIR *dirp;
    /*struct dirent {
    ino_t d_ino;  File i-node number
    char d_name[]; Null-terminated name of file
    };*/
    struct dirent *dp;
    dirp = opendir(argv[1]);
    // SYSTEM CALL FAILED
    if (!dirp) {
      fprintf(stderr, "Failed to open directory: %s\n", argv[1]);
      return -1;
    }

    while ((dp = readdir(dirp))) {
      if (active_proc >= max_proc) {
        pid = wait(&status);
        printf("Child process %d was terminated with status %d\n", pid, status);
        active_proc--;
      }
      if (is_jobs_file(dp->d_name)) {
        active_proc++;
        pid = fork();
        if (pid < 0)
          fprintf(stderr, "Failed to create a new process\n");
        if (pid == 0) { // processo filho
          exit(process(argv[1], dp->d_name));
        }
      }
    }
    while (active_proc > 0) {
      pid = wait(&status);
      printf("Child process %d was terminated with status %d\n", pid, status);
      active_proc--;
    }
  }

  ems_terminate();
  return 0;
}
