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

    int setcontext_called;

    int state;
    /* States:
     * 0: thread is unused
     * 1: thread is active
     * 2: thread is killed
     */

	ucontext_t context;

};

// Current thread is always head of the queue structure.
struct thread current_thread;


void
add_to_end(struct thread* t){
    // Function to add thread to end of queue.
    struct thread *curr = &current_thread;
    while (curr->next != NULL){
        curr = curr->next;
    }
    curr->next = t;
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

    getcontext(&(current_thread.context));
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
    printf("calling stub function\n");
        thread_main(arg); // call thread_main() function with arg
        thread_exit(0);
}

Tid
thread_create(void (*fn) (void *), void *parg)
{

    // Find an available TID
    Tid new_tid = 0;
    bool tid_unused = 0;
    while (!tid_unused){
        new_tid ++;
        if (new_tid == THREAD_MAX_THREADS){
            return THREAD_NOMORE;
        }
        tid_unused = 1;
        struct thread *curr = &current_thread;
        while (curr != NULL && tid_unused){
            if (curr->TID == new_tid) {
                tid_unused = 0;
            }
            curr = curr->next;
        }
    }

    // Allocate stack for new thread
    struct thread *new_thread = malloc(sizeof(struct thread));

    new_thread->TID = new_tid;
    new_thread->setcontext_called = 0;
    new_thread->next = NULL;
    new_thread->state = 1;
    new_thread->thread_stack = malloc(THREAD_MIN_STACK);
    getcontext(&(new_thread->context));

    // Modify the context of newly created thread
    new_thread->context.uc_mcontext.gregs[REG_RSP] = (long long int) 0;

    printf("memory address of stub function = 0x%lx\n", (unsigned long)&thread_stub);
    new_thread->context.uc_mcontext.gregs[REG_RIP] = (long long int) &thread_stub;

    printf("memory address of fn = 0x%lx\n", (unsigned long) fn);
    new_thread->context.uc_mcontext.gregs[REG_RDI] = (long long int) fn;

    printf("value of parg = 0x%lx\n", (unsigned long) parg);
    new_thread->context.uc_mcontext.gregs[REG_RSI] = (long long int) parg;

    add_to_end(new_thread);

    return new_tid;
}

Tid
thread_yield(Tid want_tid)
{
    struct thread *wanted;
    // If want_tid is THREAD_ANY or THREAD_SELF, set it to an actual TID according to requirements
    if (want_tid == THREAD_ANY || (current_thread.next != NULL && want_tid == current_thread.next->TID)){
        if (current_thread.next == NULL) {
            return THREAD_NONE;
        }
        want_tid = current_thread.next->TID;
        wanted = current_thread.next;
        add_to_end(&current_thread);

    } else if (want_tid == THREAD_SELF || want_tid == thread_id()){
        want_tid = thread_id();
        wanted = &current_thread;

    } else { // Find thread with want_tid, return THREAD_INVALID if can't find it in structure
        if ((unsigned int)want_tid >= (unsigned int)THREAD_MAX_THREADS || current_thread.next == NULL) {
            return THREAD_INVALID;
        }

        struct thread *curr = current_thread.next;
        while (curr->next != NULL && curr->next->TID != want_tid) {
            curr = curr->next;
        }
        if (curr->next == NULL) {
            return THREAD_INVALID;
        }

        // Update queue structure
        wanted = curr->next;
        curr->next = curr->next->next;
        wanted->next = current_thread.next;

        add_to_end(&current_thread);
    }

    int err = getcontext(&(current_thread.context));
    assert(!err);

    if (current_thread.setcontext_called) {
        current_thread.setcontext_called = 0;
        printf("since setcontext has been called, return the want_tid %d\n", want_tid);
        return want_tid;
    }

    current_thread.setcontext_called = 1;

    current_thread = *wanted;
    printf("since setcontext has not been called, call setcontext on thread with id %d\n", current_thread.TID);
    setcontext(&(current_thread.context));

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
