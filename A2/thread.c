#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdbool.h>
#include "thread.h"
#include "malloc369.h"
#include "interrupt.h"

/* This is the wait queue structure, needed for Assignment 2. */ 
struct wait_queue {
    struct wait_queue_node *head;
};

struct wait_queue_node {
    struct wait_queue_node *next;
    Tid tid;
};

void wait_remove(struct wait_queue *wq, Tid to_remove);

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

    struct wait_queue *sleeping_q; // This is the queue that the thread is sleeping on. NULL if it isn't waiting.

    struct wait_queue *self_q; // This is the queue that belongs to this thread.

    int state;
    /* States:
     * 0: thread is unused
     * 1: thread is active
     * 2: thread is waiting (after having called setcontext)
     * 3: thread is killed
     */

	ucontext_t context;

};

// Current thread is always head of the queue structure.
Tid current_thread = 0;
struct thread threads[THREAD_MAX_THREADS];
int exit_codes[THREAD_MAX_THREADS];

void *to_free_1 = NULL;
void *to_free_2 = NULL;

void
free_stuff(){
    if (to_free_1 != NULL){
        free369(to_free_1);
        to_free_1 = NULL;
    }
    if (to_free_2 != NULL){
        free369(to_free_2);
        to_free_2 = NULL;
    }
}

struct ready_node {
    struct ready_node *next;
    Tid tid;
};

struct ready_node *ready_head = NULL;

int ready_enqueue(Tid tid){

    struct ready_node *new_node = malloc369(sizeof(struct ready_node));
    if (new_node == NULL) {
        return 1;
    }
    new_node->next = NULL;
    new_node->tid = tid;

    if (ready_head == NULL) {
        ready_head = new_node;
    } else {
        struct ready_node *curr = ready_head;
        while (curr->next != NULL){
            curr = curr->next;
        }
        curr->next = new_node;
    }
    return 0;
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

    for (int i = 0; i < THREAD_MAX_THREADS; i++) {
        threads[i].TID = i;
        threads[i].state = 0;
        threads[i].sleeping_q = NULL;
        threads[i].self_q = NULL;
        exit_codes[i] = THREAD_INVALID;
    }

    current_thread = 0;
    threads[0].state = 1;

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
    free_stuff();
    interrupts_on();
    thread_main(arg); // call thread_main() function with arg
    interrupts_off();
    thread_exit(0);
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
    bool enabled = interrupts_off();

    // Find an available TID
    Tid new_tid = 0;
    while (threads[new_tid].state != 0){
        new_tid ++;
        if (new_tid == THREAD_MAX_THREADS){
            return THREAD_NOMORE;
        }
    }

    threads[new_tid].TID = new_tid;
    threads[new_tid].state = 1;
    threads[new_tid].thread_stack = malloc369(THREAD_MIN_STACK);
    if (threads[new_tid].thread_stack == NULL){
        return THREAD_NOMEMORY;
    }

    getcontext(&(threads[new_tid].context));

    // Modify the context of newly created thread
    threads[new_tid].context.uc_mcontext.gregs[REG_RSP] = ((long long int) threads[new_tid].thread_stack) + THREAD_MIN_STACK - 8;
    threads[new_tid].context.uc_mcontext.gregs[REG_RIP] = (long long int) &thread_stub;
    threads[new_tid].context.uc_mcontext.gregs[REG_RDI] = (long long int) fn;
    threads[new_tid].context.uc_mcontext.gregs[REG_RSI] = (long long int) parg;

    if (ready_enqueue(new_tid)) {
        free369(threads[new_tid].thread_stack);
        threads[new_tid].state = 0;
        return THREAD_NOMEMORY;
    }

    interrupts_set(enabled);
    return new_tid;
}

Tid
thread_yield(Tid want_tid)
{
    bool enabled = interrupts_off();
    struct ready_node *deleted_node = NULL;
    // If want_tid is THREAD_ANY or THREAD_SELF, set it to an actual TID according to requirements
    if (want_tid == THREAD_ANY || (ready_head != NULL && want_tid == ready_head->tid)){
        if (ready_head == NULL) {
            return THREAD_NONE;
        }
        deleted_node = ready_head;
        want_tid = ready_head->tid;
        ready_head = ready_head->next;
        ready_enqueue(thread_id());

    } else if (want_tid == THREAD_SELF || want_tid == thread_id()){
        want_tid = thread_id();
    } else { // Find thread with want_tid, return THREAD_INVALID if can't find it in structure
        if ((unsigned int)want_tid >= (unsigned int)THREAD_MAX_THREADS || ready_head == NULL || threads[want_tid].state == 0) {
            return THREAD_INVALID;
        }

        struct ready_node *curr = ready_head;
        while (curr->next != NULL && curr->next->tid != want_tid) {
            curr = curr->next;
        }
        if (curr->next == NULL) {
            return THREAD_INVALID;
        }

        // Update queue structure
        deleted_node = curr->next;
        curr->next = curr->next->next;
        ready_enqueue(thread_id());
    }
    if (deleted_node != NULL){
        free369(deleted_node);
    }

    int err = getcontext(&(threads[current_thread].context));
    assert(!err);
    free_stuff();

    if (threads[current_thread].state == 3){
        thread_exit(-SIGKILL);
    }

    if (threads[current_thread].state == 2) {
        threads[current_thread].state = 1;
        interrupts_set(enabled);
        return want_tid;
    }

    threads[current_thread].state = 2;
    current_thread = want_tid;
    setcontext(&(threads[current_thread].context));

    /* Shouldn't get here */
    interrupts_set(enabled);
	return THREAD_FAILED;
}

void
thread_exit(int exit_code)
{
    interrupts_off();
    threads[current_thread].state = 0;
    exit_codes[thread_id()] = exit_code;
    if (threads[current_thread].self_q != NULL) {
        thread_wakeup(threads[current_thread].self_q, 1);
        wait_queue_destroy(threads[current_thread].self_q);
    }
    if (ready_head == NULL){
        free_stuff();
        exit(exit_code);
    }
    if (threads[current_thread].TID != 0){
        to_free_1 = threads[current_thread].thread_stack;
    }
    current_thread = ready_head->tid;
    to_free_2 = ready_head;
    ready_head = ready_head->next;
    setcontext(&(threads[current_thread].context));
}

Tid
thread_kill(Tid tid)
{
    bool enabled = interrupts_off();
    if (tid == thread_id() || (unsigned int)tid >= (unsigned int)THREAD_MAX_THREADS || threads[tid].state == 0) {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }
	threads[tid].state = 3;
    if (threads[tid].sleeping_q != NULL) {
        wait_remove(threads[tid].sleeping_q, tid);
        ready_enqueue(tid);
    }
    interrupts_set(enabled);
    return tid;
}

/**************************************************************************
 * Important: The rest of the code should be implemented in Assignment 2. *
 **************************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc369(sizeof(struct wait_queue));
	assert(wq);


	wq->head = NULL;

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	assert(wq == NULL || wq->head == NULL);
	free369(wq);
}


void
wait_enqueue(struct wait_queue *wq, Tid tid)
{
    struct wait_queue_node *new_node = malloc369(sizeof(struct wait_queue_node));
    new_node->next = NULL;
    new_node->tid = tid;
    if (wq->head == NULL) {
        wq->head=new_node;
    } else {
        struct wait_queue_node *curr = wq->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
    threads[tid].sleeping_q = wq;
}


int
wait_dequeue(struct wait_queue *wq)
{
    if (wq->head == NULL) {
        return -1;
    }
    struct wait_queue_node *head = wq->head;
    int ret = head->tid;
    threads[head->tid].sleeping_q = NULL;
    wq->head = wq->head->next;
    free369(head);
    return ret;
}


// Assumes to_remove is in queue. Otherwise it will cause an error
void
wait_remove(struct wait_queue *wq, Tid to_remove)
{
    if (wq->head->tid == to_remove){
        wait_dequeue(wq);
    } else {
        struct wait_queue_node *curr = wq->head;
        while (curr->next->tid != to_remove){
            curr = curr->next;
        }
        struct wait_queue_node *to_free = curr->next;
        curr->next = curr->next->next;
        free369(to_free);
    }
}


Tid
thread_sleep(struct wait_queue *queue)
{
    bool enabled = interrupts_off();

    if (queue == NULL) {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }

    if (ready_head == NULL) {
        interrupts_set(enabled);
        return THREAD_NONE;
    }

    wait_enqueue(queue, current_thread);

    int ret = ready_head->tid;
    int err = getcontext(&(threads[current_thread].context));
    assert(!err);
    free_stuff();

    if (threads[current_thread].state == 3){
        thread_exit(-SIGKILL);
    }

    if (threads[current_thread].state == 2) {
        threads[current_thread].state = 1;
        interrupts_set(enabled);
        return ret;
    }
    threads[current_thread].state = 2;
    current_thread = ready_head->tid;
    to_free_1 = ready_head;
    ready_head = ready_head->next;
    setcontext(&(threads[current_thread].context));
    interrupts_set(enabled);
	return THREAD_FAILED; //Should never get here
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
    bool enabled = interrupts_off();
    if (queue == NULL || queue->head == NULL) {
        interrupts_set(enabled);
        return 0;
    }

    if (all) {
        int num = 0;
        int deq = wait_dequeue(queue);
        while (deq != -1) {
            ready_enqueue(deq);
            deq = wait_dequeue(queue);
            num ++;
        }
        interrupts_set(enabled);
        return num;
    } else {
        ready_enqueue(wait_dequeue(queue));
        interrupts_set(enabled);
        return 1;
    }
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid, int *exit_code)
{
    bool enabled = interrupts_off();
    if ((unsigned int) tid >= (unsigned int) THREAD_MAX_THREADS || tid == thread_id()) {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }
    if (threads[tid].state == 0){
        if (exit_code != NULL) {
            *exit_code = exit_codes[tid];
        }
        interrupts_set(enabled);
        return tid;
    }
	if (threads[tid].self_q == NULL) {
        threads[tid].self_q = wait_queue_create();
    }
    if (threads[tid].self_q->head != NULL) {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }
    thread_sleep(threads[tid].self_q);
    if (exit_code != NULL) {
        *exit_code = exit_codes[tid];
    }
    interrupts_set(enabled);
	return tid;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc369(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free369(lock);
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

	cv = malloc369(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free369(cv);
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
