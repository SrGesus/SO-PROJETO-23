#ifndef JOBS_H
#define JOBS_H

#include <stdint.h>

void outputFile(char *path, const char *newExtension);

void *thread_routine(void *);

/// @brief
/// @param thread_id
/// @param fd_in
/// @param fd_out
/// @return Returns -1 if there's an error parsing, 1 if success, 0 there's no
/// more to read.
intptr_t read_line(intptr_t thread_id, int fd_in, int fd_out);

#endif // JOBS_H