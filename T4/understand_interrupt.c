#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include "common.h"
#include "interrupt.h"

/*
 * In this tutorial exercise, you will think about interrupts, delivered to 
 * user mode using a timer signal.
 * You will need to understand how interrupts work, and how they interact
 * with get/setcontext to implement preemptive threading in A2.
 */

static ucontext_t my_context = { 0 };

static void
save_interrupt(void)
{
	int err;	
	volatile bool setcontext_called = false;
	
	/* QUESTION: what does interrupts_on() do? See interrupt.c */
	interrupts_on();

	/* getcontext stores the process's signal mask, as well as registers. */
	err = getcontext(&my_context);
	assert(!err);

	/* QUESTION: Are interrupts masked (i.e., disabled) in my_context?
	 * HINT: use sigismember in the print statement below. 
	 * Look at interrupt.c for examples of using sigismember.
	 */
	printf("1. in saved my_context, interrupt is disabled = %d\n", sigismember(&my_context.uc_sigmask, SIG_TYPE));

	interrupts_off();

	err = getcontext(&my_context);
	assert(!err);
	
	/* QUESTION: Are interrupts masked (i.e., disabled) in my_context now?*/
	printf("2. in saved my_context, interrupt is disabled = %d\n", sigismember(&my_context.uc_sigmask, SIG_TYPE));

	/* QUESTION: What does interrupts_enabled() do? (See interrupt.c)
	 * QUESTION: How does the process signal mask compare to the saved
	 * context? 
	 */
	printf("3. in process signal mask, interrupt is disabled = %s\n",
	       interrupts_enabled() ? "false" : "true");
	
	if (!setcontext_called) {
		interrupts_on();
		printf("4. in process signal mask, interrupt has been turned on (disabled == false).\n");
		setcontext_called = true;
		setcontext(&my_context);
	} else {
		printf("5. save_interrupt() function complete.\n\n");
		return;
	}
	
	/* QUESTION: Why can't we get to this assert? */
	assert(false);	
}

static void
test(bool enabled)
{
	int i;
	bool is_enabled;

	/* QUESTION: What is the purpose of the unintr_printf() function?
	 * More Questions: 
	 *   - What will the output of this function look like if 
	 *     interrupts are disabled when it is called?
	 *   - What will the output of this function look like if 
	 *     interrupts are enabled when it is called (and the interrupt 
	 *     handler is being loud)?
	 *   - How does the output change if the signal handler uses
	 *     write() instead of printf()? (See interrupt.c and Makefile) 
	 */ 

	for (i = 0; i < 16; i++) {
		spin(SIG_INTERVAL / 5);	/* spin for a short period */
		unintr_printf("%d ", i);
		/* Check whether interrupts are correctly enabled or not. */
		is_enabled = interrupts_enabled();
		assert(enabled == is_enabled);
	}
	unintr_printf("\n");
}

/*  test interrupts_on/off/set */
static void test_interrupts()
{
	bool enabled;

	/* No assumptions about whether interrupts are on/off upon entry. */
	enabled = interrupts_on();
	test(true);

	/* Turn off interrupts, previous state must be 'on' */
	enabled = interrupts_off();
	assert(enabled);
	test(false);

	/* Turn off interrupts again, previous state must be 'off' */
	enabled = interrupts_off();
	assert(!enabled);
	test(false);

	/* Turn on interrupts, previous state must be 'off' */
	enabled = interrupts_set(true);
	assert(!enabled);
	test(true);

	/* Turn on interrupts again, previous state must be 'on' */
	enabled = interrupts_set(true);
	assert(enabled);
	test(true);
}

int
main(int argc, char **argv)
{
	/* Install interrupt handler but don't show output when it runs. */
	register_interrupt_handler(false);

	/* See how interrupt state interacts with getcontext/setcontext. */	
	save_interrupt();

	/* Now show interrupt handler output when it runs. */
	interrupts_loud();

	/* See how interrupt handling affects the execution of other code. */
	test_interrupts();
	
	return 0;
}
