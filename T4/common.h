#ifndef _COMMON_H_
#define _COMMON_H_

#include <time.h>
#include <sys/time.h>

#define NSEC_PER_SEC 1000000000

/* Returns the result of a - b as a struct timespec. */
extern struct timespec timespec_sub(const struct timespec *a, const struct timespec *b);

extern void spin(int usecs);


#define TBD() do {							\
		printf("%s:%d: %s: please implement this functionality\n", \
		       __FILE__, __LINE__, __FUNCTION__);		\
		exit(1);						\
	} while (0)


#endif /* _COMMON_H_ */
