#ifndef SEAT_H
#define SEAT_H

#include <stddef.h>

typedef struct seat {
    size_t x;
    size_t y;
} seat_t;

/// @param a first seat
/// @param b second seat
/// @return Positive if a > b, 0 if a == b, negative if a < b
int seat_cmp(const void *a, const void*b);

/// Quicksort for seats.
/// @param seats Pointer to the array to store
/// @param count Num of seats.
void seat_sort(seat_t *seats, size_t count);

#endif //SEAT_H