#ifndef COMMON_IO_H
#define COMMON_IO_H

#include <stddef.h>

/// Parses an unsigned integer from the given file descriptor.
/// @param fd The file descriptor to read from.
/// @param value Pointer to the variable to store the value in.
/// @param next Pointer to the variable to store the next character in.
/// @return 0 if the integer was read successfully, 1 otherwise.
int parse_uint(int fd, unsigned int *value, char *next);

/// Prints an unsigned integer to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param value The value to write.
/// @return 0 if the integer was written successfully, 1 otherwise.
int print_uint(int fd, unsigned int value);

/// Writes a string to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param str The string to write.
/// @return 0 if the string was written successfully, 1 otherwise.
int print_str(int fd, const char *str);

/// Writes a n chars to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param len Count of bytes to write.
/// @param buf The buffer to write
/// @return 0 if the string was written successfully, 1 otherwise.
int write_nstr(int fd, size_t len, void *buf);

/// Writes an unsigned integer to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param value Value to be written
/// @return 0 if the integer was read successfully, 1 otherwise.
int write_uint(int fd, unsigned int value);

/// Writes an integer to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param value Value to be written
/// @return 0 if the integer was read successfully, 1 otherwise.
int write_int(int fd, int value);

/// Reads an unsigned integer from the given file descriptor.
/// @param fd The file descriptor to read from.
/// @param value Pointer to the variable to store the value in.
/// @return 0 if the integer was read successfully, 1 otherwise.
int read_uint(int fd, unsigned int *value);

/// Reads an integer from the given file descriptor.
/// @param fd The file descriptor to read from.
/// @param value Pointer to the variable to store the value in.
/// @return 0 if the integer was read successfully, 1 otherwise.
int read_int(int fd, int *value);

/// Writes a size_t variable to the given file descriptor.
/// @param fd The file descriptor to write to.
/// @param value Value to be written
/// @return 0 if the integer was read successfully, 1 otherwise.
int write_size(int fd, size_t value);

/// Reads a size_t variable from the given file descriptor.
/// @param fd The file descriptor to read from.
/// @param value Pointer to the variable to store the value in.
/// @return 0 if the integer was read successfully, 1 otherwise.
int read_size(int fd, size_t *value);

#endif  // COMMON_IO_H
