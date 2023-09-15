/* 
 * Self-tester for Tutorial 1, Fall 2023.
 *
 * Use this file to write your own tests for the queue operations. 
 *
 */

#include <ctype.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include "queue.h"

FILE *log_file;


/* Structure to hold pointers to queue functions.
 * This lets us use the same test code for either queue implementation,
 * by passing either a structure initialized with all the queue_A functions,
 * or a structure initialized with all the queue_B functions. */

struct queue_X {
	char *name;
	int (*initialize)(void);
	int (*enqueue)(void *);
	int (*dequeue)(void **);
	int (*remove_from_queue)(void *);
	void (*print_queue)(void);
	int (*destroy)(void);
};

/* Pointers to the queue_A functions */

struct queue_X queue_A = {
	.name = "queue_A",
	.initialize = queue_A_initialize,
	.enqueue = queue_A_enqueue,
	.dequeue = queue_A_dequeue,
	.remove_from_queue = queue_A_remove_from_queue,
	.print_queue = queue_A_print_queue,
	.destroy = queue_A_destroy
};

struct queue_X queue_B = {
	.name = "queue_B",
	.initialize = queue_B_initialize,
	.enqueue = queue_B_enqueue,
	.dequeue = queue_B_dequeue,
	.remove_from_queue = queue_B_remove_from_queue,
	.print_queue = queue_B_print_queue,
	.destroy = queue_B_destroy
};
	

void usage(char *progname)
{
	fprintf(log_file,
		"Usage: %s: \n\t[-p (a|b|ab) select parts] \n",	progname);
	exit(1);

}

void parse_args(int argc, char **argv, bool *test_part_a, bool *test_part_b)
{
	int opt;

	if (argc < 2) {
		usage(argv[0]);
	}
	
	while((opt = getopt(argc, argv, "p:")) != -1){
		switch (opt) {
		case 'p':
		{
			char part_string[4];
			strncpy(part_string, optarg, 2);
			if(strlen(part_string) == 0) {
				printf("zero length part string\n");
				usage(argv[0]);
			}
			for(int i = 0; i < strlen(part_string); i++) {
				switch (part_string[i])	{
				case 'a':
					*test_part_a = true;
					break;
				case 'b':
					*test_part_b = true;
					break;
				default:
					printf("invalid part %c\n", part_string[i]);
					usage(argv[0]);
				}
			}
			break;
		}
		default:
			usage(argv[0]);
		}
	}
}


void run_tests(struct queue_X *the_queue)
{
	int err;
	void *item;

	/* Initialize the queue. */
	if ((err = the_queue->initialize()) != 0) {
		fprintf(log_file, "%s returned error %d from initialize.\n",
			the_queue->name, err);
		return;
	}

	/* Test enqueue of a single item. */
	if ((err = the_queue->enqueue((void *)0xc5c369)) != 0) {
		fprintf(log_file, "%s returned error %d from enqueue.\n",
			the_queue->name, err);
		return;
	}

	/* Test printing the queue. */
	the_queue->print_queue();

	/* Test dequeue of the item previously enqueued. */
	if ((err = the_queue->dequeue(&item)) != 0) {
		fprintf(log_file, "%s returned error %d from dequeue.\n",
			the_queue->name, err);
		return;
	}

	if (item != (void *)0xc5c369) {
		fprintf(log_file, "%s dequeued item is %p, expected %p.\n",
			the_queue->name, item, (void *)0xc5c369);
		return;
	}

	/* Check if value of dequeued item matches value that was enqueued. */
	if ((err = the_queue->destroy()) != 0) {
		fprintf(log_file, "%s returned error %d from destroy.\n",
			the_queue->name, err);
		return;
	}
		
	return;
}

int main(int argc, char ** argv)
{
	bool test_part_a = false;
	bool test_part_b = false;

	log_file = stdout;
	parse_args(argc, argv, &test_part_a, &test_part_b);

	if (test_part_a) {
		run_tests(&queue_A);
	}
	
	if (test_part_b) {
		run_tests(&queue_B);
	}

	return 0;
}
