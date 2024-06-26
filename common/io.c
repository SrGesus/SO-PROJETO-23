#include "io.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parse_uint(int fd, unsigned int *value, char *next) {
  char buf[16];

  int i = 0;
  while (1) {
    ssize_t read_bytes = read(fd, buf + i, 1);
    if (read_bytes == -1) {
      return 1;
    } else if (read_bytes == 0) {
      *next = '\0';
      break;
    }

    *next = buf[i];

    if (buf[i] > '9' || buf[i] < '0') {
      buf[i] = '\0';
      break;
    }

    i++;
  }

  unsigned long ul = strtoul(buf, NULL, 10);

  if (ul > UINT_MAX) {
    return 1;
  }

  *value = (unsigned int)ul;

  return 0;
}

int print_uint(int fd, unsigned int value) {
  char buffer[16];
  size_t i = 16;

  for (; value > 0; value /= 10) {
    buffer[--i] = '0' + (char)(value % 10);
  }

  if (i == 16) {
    buffer[--i] = '0';
  }

  while (i < 16) {
    ssize_t written = write(fd, buffer + i, 16 - i);
    if (written == -1) {
      return 1;
    }

    i += (size_t)written;
  }

  return 0;
}

int print_str(int fd, const char *str) {
  size_t len = strlen(str);
  while (len > 0) {
    ssize_t written = write(fd, str, len);
    if (written == -1) {
      return 1;
    }

    str += (size_t)written;
    len -= (size_t)written;
  }

  return 0;
}

int write_uint(int fd, unsigned int value) {
  const size_t BUFFER_SIZE = sizeof(unsigned int);
  char buffer[BUFFER_SIZE];
  memcpy(buffer, &value, BUFFER_SIZE);
  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t written = write(fd, (void *)(buffer + i), BUFFER_SIZE - i);
    if (written == -1) {
      return 1;
    }
    i += (size_t)written;
  }
  return 0;
}

int write_int(int fd, int value) {
  const size_t BUFFER_SIZE = sizeof(int);
  char buffer[BUFFER_SIZE];
  memcpy(buffer, &value, BUFFER_SIZE);
  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t written = write(fd, buffer + i, BUFFER_SIZE - i);
    if (written == -1) {
      return 1;
    }
    i += (size_t)written;
  }
  return 0;
}

int write_size(int fd, size_t value) {
  const size_t BUFFER_SIZE = sizeof(size_t);
  char buffer[BUFFER_SIZE];
  memcpy(buffer, &value, BUFFER_SIZE);
  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t written = write(fd, buffer + i, BUFFER_SIZE - i);
    if (written == -1) {
      return 1;
    }
    i += (size_t)written;
  }
  return 0;
}

int read_uint(int fd, unsigned int *value) {
  const size_t BUFFER_SIZE = sizeof(unsigned int);
  char buffer[BUFFER_SIZE];

  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t read_bytes = read(fd, buffer + i, BUFFER_SIZE - i);
    if (read_bytes == -1) {
      return 1;
    }
    i += (size_t)read_bytes;
  }

  memcpy(value, buffer, BUFFER_SIZE);

  // printf("Read: %u\n", *value);

  return 0;
}
int read_int(int fd, int *value) {
  const size_t BUFFER_SIZE = sizeof(int);
  char buffer[BUFFER_SIZE];

  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t read_bytes = read(fd, buffer + i, BUFFER_SIZE - i);
    if (read_bytes == -1) {
      return 1;
    }
    i += (size_t)read_bytes;
  }

  memcpy(value, buffer, BUFFER_SIZE);

  return 0;
}

int read_size(int fd, size_t *value) {
  const size_t BUFFER_SIZE = sizeof(size_t);
  char buffer[BUFFER_SIZE];

  size_t i = 0;
  while (i < BUFFER_SIZE) {
    ssize_t read_bytes = read(fd, buffer + i, BUFFER_SIZE - i);
    if (read_bytes == -1) {
      return 1;
    }
    i += (size_t)read_bytes;
  }
  memcpy(value, buffer, BUFFER_SIZE);

  // printf("Read: %lu\n", *value);

  return 0;
}

int write_nbytes(int fd, void *buf, size_t len) {
  while (len > 0) {
    ssize_t written = write(fd, buf, len);
    if (written == -1) {
      return 1;
    }

    buf += (size_t)written;
    len -= (size_t)written;
  }

  return 0;
}

int read_nbytes(int fd, void *buf, size_t len) {
  size_t i = 0;
  while (i < len) {
    ssize_t read_bytes = read(fd, buf + i, len - i);
    if (read_bytes == -1) {
      return 1;
    }
    i += (size_t)read_bytes;
  }

  return 0;
}
