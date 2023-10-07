#include <assert.h>
#include "common.h"

#define NSEC_PER_SEC 1000000000

/* Returns the result of a - b as a struct timespec. */
struct timespec
timespec_sub(const struct timespec *a, const struct timespec *b)
{
	struct timespec r;
	r.tv_sec = a->tv_sec - b->tv_sec;
	r.tv_nsec = a->tv_nsec - b->tv_nsec;
	if (r.tv_nsec < 0) {
		/* borrow from secs */
		r.tv_sec--;
		r.tv_nsec += NSEC_PER_SEC;
	}
	return r;
}

/* Busy wait for the given number of microseconds */
void
spin(int usecs)
{
	struct timespec start, end, diff;
	int ret;
	int nsecs = usecs * 1000;
	
	ret = clock_gettime(CLOCK_REALTIME, &start);
	assert(!ret);
	while (1) {
		ret = clock_gettime(CLOCK_REALTIME, &end);
		diff = timespec_sub(&end, &start);

		if ((diff.tv_sec * NSEC_PER_SEC + diff.tv_nsec) >= nsecs) {
			break;
		}
	}
}
