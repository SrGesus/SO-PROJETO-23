#ifndef JOBS_H
#define JOBS_H

#include "parser.h"
#include "operations.h"
#include "constants.h"

void outputFile(char * path, const char *  newExtension);

int read_batch(int fd_in, int fd_out);

#endif // JOBS_H