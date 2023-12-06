#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"
#include "reader.h"
#include "write.h"

int main(int argc, char *argv[]) {
  //neste momento leva um delay em ms mas agora leva o dirPath tbm
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;

  if (argc > 2){
    char *endptr;
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

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

  if (argc > 1){
    int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
    S_IROTH | S_IWOTH;
    char* total_path;
    DIR *dirp;
  
    /*struct dirent {
    ino_t d_ino;  File i-node number
    char d_name[]; Null-terminated name of file 
    };*/
    struct dirent *dp;
    dirp = opendir(argv[1]);
  //SYSTEM CALL FAILED
    if (!dirp){ 
      fprintf(stderr,"opendir failed on %s",argv[1]);
      return -1;
    }
    dp = readdir(dirp);
    while (dp){
      if (strcmp(dp->d_name,".") && strcmp(dp->d_name,"..") && !strcmp(strrchr(dp->d_name, '.'), ".jobs")){
        total_path = (char*)malloc((strlen(dp->d_name)+strlen(argv[1])+2)*sizeof(char));
        strcpy(total_path,argv[1]);
        strcat(total_path,"/");
        strcat(total_path,dp->d_name);  // total_path = jobs/job.jobs
        int fd_in = open(total_path, O_RDONLY);
        if (fd_in < 0) {
          fprintf(stderr, "Could not open file: %s", total_path);
        } else {
          outputFile(total_path,".out"); // total_path = jobs/job.out
          int fdout = open(total_path, openFlags, filePerms);
          if (fdout < 0) {
            fprintf(stderr, "Could not open file: %s", total_path);
          } else {
            while(read_batch(fd_in,fdout) != 1); // read_bach returns 1 when it reaches EOC
            fsync(fdout);
            close(fd_in);
            close(fdout);
          }
        }
        free(total_path);
      }
      dp = readdir(dirp);
    }
    ems_terminate();
    exit(0);
  }

  while(1) {
    printf("> ");
    read_batch(STDIN_FILENO, STDOUT_FILENO);
  }
}
