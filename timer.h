#ifndef TIMEOUT_H
#define TIMEOUT_H
#include <time.h>
#include <math.h>
#include <stdint.h>

#define UNIT_TO_NANO(value) ((value)*1000000000)
#define UNIT_TO_MICRO(value) ((value)*1000000)

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


static inline void timer_sleep(float seconds)
{
    struct timespec req;
    struct timespec rem;
    int rv;

    req.tv_sec = trunc(seconds);
    req.tv_nsec = UNIT_TO_NANO(seconds - req.tv_sec);

    do{
        rv = nanosleep(&req, &rem);
        req = rem;
    }while(rv != 0);
}
#endif /* TIMEOUT_H */
