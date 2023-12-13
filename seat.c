
#include "seat.h"
#include <stdlib.h>

int seat_cmp(const void *a, const void*b) {
    seat_t seat_a = *(seat_t *)a;
    seat_t seat_b = *(seat_t *)b;
    if (seat_a.x == seat_b.x)
        return (int)(seat_a.y - seat_b.y);
    else
        return (int)(seat_a.x - seat_b.x);
}

void seat_sort(seat_t *seats, size_t count) {
    qsort((void *)seats, count, sizeof(seat_t), seat_cmp);
}