#ifndef WRITE_H
#define WRITE_H

#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

int write_fmt(int fd, const char * fmt, ...);

#endif // WRITE_H