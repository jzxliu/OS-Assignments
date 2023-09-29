#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdbool.h>
#include "thread.h"

/* This is the wait queue structure, needed for Assignment 2. */ 
struct wait_queue {
	/* ... Fill this in Assignment 2 ... */
};

/* For Assignment 1, you will need a queue structure to keep track of the 
 * runnable threads. You can use the tutorial 1 queue implementation if you 
 * like. You will probably find in Assignment 2 that the operations needed
 * for the wait_queue are the same as those needed for the ready_queue.
 */

/* This is the thread control block. */
struct thread {

    Tid TID;

    /* Points to the thread stack allocated*/
    void *thread_stack;

    struct thread *next;

    volatile int setcontext_called;

    int state;
    /* States:
     * 0: thread is unused
     * 1: thread is active
     * 2: thread is killed
     */

	ucontext_t context;

};

struct thread current_thread;


/**************************************************************************
 * Assignment 1: Refer to thread.h for the detailed descriptions of the six
 *               functions you need to implement.
 **************************************************************************/

void
thread_init(void)
{
	/* Add necessary initialization for your threads library here. */
    /* Initialize the thread control block for the first thread */

    getcontext(&(current_thread.context));
    current_thread.thread_stack = current_thread.context.uc_mcontext.gregs[RSP];
    current_thread.setcontext_called = 0;
    current_thread.TID = 0;
    current_thread.next = NULL;
    current_thread.state = 1;

}

Tid
thread_id()
{
	return current_thread.TID;
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function.
 */
void
thread_stub(void (*thread_main)(void *), void *arg)
{
        thread_main(arg); // call thread_main() function with arg
        thread_exit(0);
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	/* 1. Find an available TID
	 * 2. Allocate space for its context, make new context
	 * 3. Allocate stack for new thread
	 * 4. Change its context according to assignment requirements (RSP - Stack Pointer, RIP - Instruction Pointer)
	 * 5.
	 */
    /*Tid new_tid = 0;
    while (threads[new_tid].state != 0){
        new_tid ++;
        if (new_tid == THREAD_MAX_THREADS){
            return THREAD_NOMORE;
        }
    }

    threads[new_tid].context = malloc(sizeof(ucontext_t));

    getcontext(threads[new_tid].context);

    threads[new_tid].thread_stack = malloc(THREAD_MIN_STACK);

    threads[new_tid].context.uc_mcontext.gregs[RSP] = threads[new_tid].thread_stack;

    threads[new_tid].context->uc_mcontext.gregs[RIP] = &thread_stub();

    return new_tid;*/
    return 0;
}

Tid
thread_yield(Tid want_tid)
{
    // If want_tid is THREAD_ANY or THREAD_SELF, set it to an actual TID according to requirements
    if (want_tid == THREAD_ANY){
        if (ready_size <= 1) {
            return THREAD_NONE;
        }
        want_tid = current_thread.next->TID;
    } else if (want_tid == THREAD_SELF){
        want_tid = thread_id();
    } else {
        if ((unsigned int)want_tid >= (unsigned int)THREAD_MAX_THREADS) {
            return THREAD_INVALID;
        }
    }

    // Find thread with want_tid, return THREAD_INVALID if can't find it in structure
    struct thread wanted = current_thread;
    if (want_tid != thread_id()) {
        while (wanted.next != NULL && wanted.next.TID != want_tid) {
            wanted = wanted.next;
        }
    }

    // to do: Update queue structure

    int err = getcontext(current_thread.context);
    assert(!err);

    if (current_thread.setcontext_called) {
        current_thread.setcontext_called = 0;
        return want_tid;
    }

    current_thread.setcontext_called = 1;
    setcontext(wanted.context);

    /* Shouldn't get here */
	return THREAD_FAILED;
}

void
thread_exit(int exit_code)
{
	TBD();
}

Tid
thread_kill(Tid tid)
{
	TBD();
	return THREAD_FAILED;
}

/**************************************************************************
 * Important: The rest of the code should be implemented in Assignment 2. *
 **************************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);

	TBD();

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	TBD();
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	TBD();
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid, int *exit_code)
{
	TBD();
	return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}
