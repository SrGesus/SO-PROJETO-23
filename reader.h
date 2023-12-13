#ifndef JOBS_H
#define JOBS_H

#include <stdint.h>

void outputFile(char *path, const char *newExtension);

void *thread_routine(void *);

intptr_t read_line(intptr_t thread_id, int fd_in, int fd_out);

#endif // JOBS_H