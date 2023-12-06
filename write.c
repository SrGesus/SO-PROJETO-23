
#include "write.h"

int write_fmt(int fd, const char * fmt, ...) {
  const size_t buffer_size = 256;
  char buffer[buffer_size];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buffer, buffer_size, fmt, args);
  va_end(args);
  size_t len = strlen(buffer);
  size_t done = 0;

  while (len > 0) {
    ssize_t bytes_written = write(fd, buffer + done, len);

    if (bytes_written < 0){
      fprintf(stderr, "Write error: %s\n", strerror(errno));
      return -1;
    }

    /* might not have managed to write all, len becomes what remains */
    len -= (size_t)bytes_written;
    done += (size_t)bytes_written;
  }
  return 0;
}
