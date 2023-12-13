#ifndef JOBS_H
#define JOBS_H

void outputFile(char *path, const char *newExtension);

void * thread_routine(void *);

int read_batch(int fd_in, int fd_out);

#endif // JOBS_H