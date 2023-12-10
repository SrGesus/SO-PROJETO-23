#ifndef WRITE_H
#define WRITE_H

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int write_fmt(int fd, const char *fmt, ...);

#endif // WRITE_H