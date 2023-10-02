/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Eric Munson, Angela Demke Brown
 * Copyright (c) 2023 Eric Munson, Angela Demke Brown
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <pthread.h>
#include "sync.h"
#include "output.h"

/*
 * This program uses the barrier abstraction to synchronize a set of threads.
 * The threads all execute some number of "phases" and must wait for each other
 * at the barrier point at the end of each phase before they can proceed to the 
 * next phase. 
 *  
 * You will need to add the code to the barrier structure, as well as the 
 * barrier_init() and barrier() functions to ensure that all the prints from 
 * phase N appear before any of the prints from phase N+1.
 *  
 * There should be no enforced order on the prints from different threads in the
 * same phase. 
 *
 * For example, the following output for 4 threads and 2 phases are both allowed: 
 *
 * Legal output 1:                   Legal output 2:
 * thread 2 in phase 0.              thread 0 in phase 0.     
 * thread 1 in phase 0.		     thread 2 in phase 0.     
 * thread 0 in phase 0.		     thread 1 in phase 0.     
 * thread 3 in phase 0.		     thread 3 in phase 0.     
 * thread 2 in phase 1.		     thread 2 in phase 1.     
 * thread 1 in phase 1.		     thread 0 in phase 1.     
 * thread 0 in phase 1.		     thread 3 in phase 1.     
 * thread 3 in phase 1.		     thread 1 in phase 1.     
 * thread 1 done all phases.	     thread 2 done all phases.
 * thread 0 done all phases.	     thread 1 done all phases.
 * thread 3 done all phases.	     thread 0 done all phases.
 * thread 2 done all phases.	     thread 3 done all phases.
 * Barrier test complete.	     Barrier test complete.   
 *
 * All your changes should be made in this file.
 * You should not need to add any global variables, except by adding fields to 
 * struct barrier_s. 
 * You should not need to alter any functions except barrier_init() and 
 * barrier() to solve the problem.
 */

int Nthreads; /* Number of threads in test */
int Nphases;  /* Number of phases */


struct barrier_s {
	int num_arrived; /* Number of threads that have arrived at barrier */
	bool ready;      /* True if barrier is ready for threads to arrive */

    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

};

struct barrier_s bar; /* Global barrier for threads to wait between phases */


void barrier_init()
{
	bar.num_arrived = 0;
	bar.ready = false;

}

/* Reusable barrier. A calling thread must wait at the barrier until all 
 * Nthreads threads have arrived at the barrier. The last thread to arrive should
 * release all the other threads so that they can proceed past the barrier. 
 *
 * It must be possible for the threads to reuse the same barrier to synchronize 
 * with each other multiple times. 
 *
 * You may assume that pthread_cond_wait() will not return unless either 
 * pthread_cond_signal() or pthread_cond_broadcast() is called on the 
 * condition variable.
 */
void barrier()
{
    mutex_lock(&(bar.mutex));
    bar.num_arrived ++;
    if (bar.num_arrived == Nthreads) {
        pthread_cond_broadcast(&(bar.cv));
    }
    pthread_cond_wait(&(bar.cv), &(bar.mutex));
    bar.num_arrived --;
    if (bar.num_arrived == 0) {
        pthread_cond_broadcast(&(bar.cv));
    }
    pthread_cond_wait(&(bar.cv), &(bar.mutex));
    mutex_unlock(&(bar.mutex));

}

/****************************************************************************
 *
 * You should not change anything below this point.
 *
 ****************************************************************************/

/* This is the function that each thread executes. 
 * Threads execute Nphases iterations of the loop, printing their output and
 * then waiting at the barrier for all threads to finish the current phase.
 */
void *thread_func(void *arg)
{
	long me = (long)arg;

	for (int i = 0; i < Nphases; i++) {
		print_phase(me, i);
		barrier();
	}

	print_done(me);
	return NULL;
}


int main(int argc, char **argv)
{
	pthread_t *threads; /* array for pthread ids set by pthread_create */

	/* Check arguments and set up number of threads and phases for test */
	if (argc != 3) {
		printf("Usage: ./barrier <Nthreads> <Nphases>\n");
		printf("\t(using defaults)\n");
		Nthreads = 4;
		Nphases =  3;
	} else {
		Nthreads = atoi(argv[1]);
		Nphases = atoi(argv[2]);
	}	
	printf("Barrier test starting. Nthreads = %d, Nphases = %d\n",
	       Nthreads, Nphases);

	/* Allocate memory for pthread ids and initialize output functions */
	threads = (pthread_t *)malloc(Nthreads*sizeof(pthread_t));
	output_init();

	/* Initialize the barrier */
	barrier_init();

	/* Create the threads */
	for (long i = 0; i < Nthreads; i++) {
		if (pthread_create(&threads[i], NULL, thread_func, (void *)i)) {
			perror("pthread_create");
		}
	}

	/* Wait for all the threads to finish */
	for (long i = 0; i < Nthreads; i++) {
		if (pthread_join(threads[i], NULL)) {
			perror("pthread_join");
		}
	}

	printf("Barrier test complete.\n");
	return 0;
}

