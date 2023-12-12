#ifndef SEAT_H
#define SEAT_H

#include <stddef.h>
#include <stdlib.h>

typedef struct seat {
    size_t x;
    size_t y;
} seat_t;

/// @param a first seat
/// @param b second seat
/// @return Positive if a > b, 0 if a == b, negative if a < b
int seat_cmp(const void *a, const void*b) {
    seat_t seat_a = *(seat_t *)a;
    seat_t seat_b = *(seat_t *)b;
    if (seat_a.x == seat_b.x)
        return (int)(seat_a.y - seat_b.y);
    else
        return (int)(seat_a.x - seat_b.x);
}

/// Quicksort for seats.
/// @param seats Pointer to the array to store
/// @param count Num of seats.
void seat_sort(seat_t *seats, size_t count) {
    qsort((void *)seats, count, sizeof(seat_t), seat_cmp);
}

#endif //SEAT_H