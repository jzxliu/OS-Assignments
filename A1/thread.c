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

    volatile int setcontext_called;

    int state;
    /* States:
     * 0: thread is unused
     * 1: thread is running
     * 2: thread is waiting
     */

    bool killed;
	ucontext_t context;

};

bool threads_initialized = false;

struct thread threads[THREAD_MAX_THREADS];

volatile Tid current_thread;


typedef struct ready_queue_node {

    Tid TID;

} ready_node;

volatile ready_node ready_queue[THREAD_MAX_THREADS];
volatile unsigned long ready_head = 0;
volatile unsigned long ready_tail = 0;
volatile unsigned long ready_size = 0;


/* Add thread to the tail of the queue.
 * Returns 0 on success or an appropriate error code on failure.
 */
int ready_queue_enqueue(Tid TID)
{

    ready_queue[ready_tail].TID = TID;

    ready_tail += 1;
    if (ready_tail >= THREAD_MAX_THREADS) {
        ready_tail = 0;
    }

    ready_size += 1;

    return 0;
}

/* Returns the first Tid in the queue, and remove it from the queue.
 * Returns -1 if queue is empty
 */
Tid ready_queue_pop()
{
    if(ready_size == 0){
        return -1;
    }
    Tid out = ready_queue[ready_head].TID;
    ready_head += 1;
    if (ready_head >= THREAD_MAX_THREADS) {
        ready_head = 0;
    }
    ready_size -= 1;
    return out;
}


/* Search the queue for 'TID' and, if found, remove it from the queue.
 * Returns 0 if the TID is found, or -1 if the TID is not in the queue.
 */
int ready_queue_remove(Tid TID)
{
    unsigned long curr = ready_head;
    while (curr != ready_tail) {
        if (ready_queue[curr].TID == TID){
            ready_tail -= 1;
            if (ready_tail <= -1){
                ready_tail = THREAD_MAX_THREADS - 1;
            }
            while (curr != ready_tail) {
                if (curr != THREAD_MAX_THREADS - 1) {
                    ready_queue[curr].TID = ready_queue[curr+1].TID;
                } else {
                    ready_queue[curr].TID = ready_queue[0].TID;
                }

                curr ++;
                if (curr >= THREAD_MAX_THREADS) {
                    curr = 0;
                }
            }
            ready_size -= 1;
            return 0;
        }
        curr ++;
        if (curr >= THREAD_MAX_THREADS) {
            curr = 0;
        }
    }
    return -1;
}

/**/
static void
call_setcontext(ucontext_t * context)
{
    int err = setcontext(context);
    assert(!err);
}

static void
call_getcontext(ucontext_t * context)
{
    int err = getcontext(context);
    assert(!err);
}

/**************************************************************************
 * Assignment 1: Refer to thread.h for the detailed descriptions of the six
 *               functions you need to implement.
 **************************************************************************/

void
thread_init(void)
{
	/* Add necessary initialization for your threads library here. */
        /* Initialize the thread control block for the first thread */
	if (threads_initialized) {
        return;
    }
    current_thread = 0;
    threads[0].state = 1;
    threads[0].setcontext_called = 0;
    threads[0].TID = 0;
    threads[0].killed = 0;
}

Tid
thread_id()
{
	return current_thread;
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
	TBD();
	return THREAD_FAILED;
}

Tid
thread_yield(Tid want_tid)
{

    if (want_tid == THREAD_ANY){
        if (ready_size <= 1) {
            return THREAD_NONE;
        }
        want_tid = ready_queue[ready_head].TID;
        ready_queue_remove(want_tid);
        ready_queue_enqueue(current_thread);
    } else if (want_tid == THREAD_SELF){
        want_tid = current_thread;
    } else {
        if (threads[want_tid].state == 0) {
            return THREAD_INVALID;
        }

        /* modify the queue */
        ready_queue_remove(want_tid);
        ready_queue_enqueue(current_thread);
    }

    call_getcontext(&(threads[current_thread].context));

    if (threads[current_thread].setcontext_called) {
        threads[current_thread].setcontext_called = 0;
        return want_tid;
    }

    threads[current_thread].setcontext_called = 1;
    current_thread = want_tid;
    call_setcontext(&(threads[want_tid].context));

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
