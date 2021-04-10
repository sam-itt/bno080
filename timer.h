#ifndef TIMEOUT_H
#define TIMEOUT_H
#include <time.h>
#include <stdint.h>

typedef struct timespec Timer;

static inline int timer_start(Timer *self)
{
    return clock_gettime(CLOCK_MONOTONIC, self);
}

/*Timeout in seconds*/
static inline bool timeout_elapsed(Timer *reference, uint32_t timeout)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    return (now.tv_sec - reference->tv_sec) > timeout;
}

#endif /* TIMEOUT_H */
