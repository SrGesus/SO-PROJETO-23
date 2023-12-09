#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"
#include "reader.h"
#include "write.h"

int main(int argc, char *argv[]) {
  //neste momento leva um delay em ms mas agora leva o dirPath tbm
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;
  int max_proc, estado;
  pid_t pid; //init_pid = getpid();
  if (argc > 3){
    char *endptr;
    unsigned long int delay = strtoul(argv[3], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_ms = (unsigned int)delay;
  }
  if (argc > 2) max_proc = atoi(argv[2]);

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
    dirp = opendir(argv[1]);
    struct dirent *dp;
    //SYSTEM CALL FAILED
    if (!dirp){ 
      fprintf(stderr,"opendir failed on %s",argv[1]);
      return -1;
    }
    while ((dp = readdir(dirp)) && max_proc > 0){
      if (strcmp(dp->d_name,".") && strcmp(dp->d_name,"..") && !strcmp(strrchr(dp->d_name, '.'), ".jobs")){
          pid = fork();
          if (pid < 0) fprintf(stderr, "Could not create a new process\n");
          if (pid == 0){ // processo filho
            total_path = (char*)malloc((strlen(dp->d_name)+strlen(argv[1])+2)*sizeof(char));
            strcpy(total_path,argv[1]);
            strcat(total_path,"/");
            strcat(total_path,dp->d_name);  // total_path = jobs/job.jobs
            printf("Doing %s with %d\n",total_path,max_proc);
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
            max_proc--;
            if (max_proc == 0) exit(0);
          }
          else{
            pid = wait(&estado);
            printf("Parent process %d was terminated with status %d\n",pid, estado);
            exit(estado);
          }
    }
  }
  ems_terminate();
  exit(0);
  }
}
