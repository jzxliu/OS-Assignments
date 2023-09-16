#include <assert.h>
#include "queue.h"

/*************************************************
 * Implement the following functions.
 * DO NOT add any global (static) variables,
 * Except in Part C.
 * You can add help functions as you need.
 ************************************************/


/*************************************************
 * ------------------Part A:---------------------- 
 * In this part, you will implement
 * a  dynamically allocated queue with
 * a linked list. 
 * DO NOT add any global (static) variables.
 *************************************************/

bool queue_A_initialized = false;

typedef struct queue_A_node {

/* Add code BEGIN */
    struct queue_A_node *next;

/* Add code END */

	void *item;

} queue_A_node_t;

queue_A_node_t *queue_A_head = NULL;
queue_A_node_t *queue_A_tail = NULL;

/* Add code BEGIN */

/* Helper functions and macros only! */



/* Add code END */

/* Perform any initialization needed so that the queue data structure can be 
 * used. 
 * Returns 0 on success or an error if the queue has already been initialized.
 */
int queue_A_initialize()
{

/* Add code BEGIN */
    if (queue_A_initialized){
        return ERR_INITIALIZED;
    }
    queue_A_initialized = true;

	return 0;
/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Add item to the tail of the queue.
 * Returns 0 on success or an appropriate error code on failure.
 */
int queue_A_enqueue(void *item)
{

/* Add code BEGIN */
    if (!queue_A_initialized){
        return ERR_NOT_INITIALIZED;
    }
    queue_A_node_t *new_node = malloc(sizeof(queue_A_node_t));
    new_node->item = item;
    new_node->next = NULL;
    if (queue_A_tail != NULL){
        queue_A_tail->next = new_node;
    } else {
        queue_A_head = new_node;
    }
    queue_A_tail = new_node;
    return 0;
	
/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Remove the item at the head of the queue and store it in the location
 * pointed to by 'item'.
 * Returns 0 on success or an appropriate error code on failure.
 */
int queue_A_dequeue(void **item)
{

/* Add code BEGIN */

    if (!queue_A_initialized){
        return ERR_NOT_INITIALIZED;
    }

    if (queue_A_head == NULL){
        return ERR_EMPTY;
    }

    if (item == NULL){
        return ERR_INVALID_ARG;
    }

    *item = queue_A_head->item;
    queue_A_node_t* to_be_freed = queue_A_head;
    queue_A_head = queue_A_head->next;
    if (queue_A_head == NULL) {
        queue_A_tail = NULL;
    }
	free(to_be_freed);
    return 0;

/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Search the queue for 'item' and, if found, remove it from the queue.
 * Returns 0 if the item is found, or an error code if the item is not 
 * in the queue. 
 */
int queue_A_remove_from_queue(void *item)
{

/* Add code BEGIN */
    if (!queue_A_initialized){
        return ERR_NOT_INITIALIZED;
    }

    if (queue_A_head == NULL){
        return ERR_NO_SUCH_ITEM;
    }

    if (queue_A_head->item == item){
        queue_A_node_t* to_be_freed = queue_A_head;
        queue_A_head = queue_A_head->next;
        if (queue_A_head == NULL) {
            queue_A_tail = NULL;
        }
        free(to_be_freed);
        return 0;
    }

    queue_A_node_t *curr = queue_A_head;

    while(curr->next != NULL){
        if (curr->next->item == item){
            queue_A_node_t* to_be_freed = curr->next;
            curr->next = curr->next->next;
            if (curr->next == NULL){
                queue_A_tail = curr;
            }
            free(to_be_freed);
            return 0;
        }
        curr = curr->next;
    }

    return ERR_NO_SUCH_ITEM;

/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Print the contents of the queue from head to tail.
 * Do not change the existing printf's, outside of the Add BEGIN/END markers.
 * Refer to tutorial handout for expected output format. 
 */
void queue_A_print_queue()
{
	printf("head: %p, tail: %p\n", queue_A_head, queue_A_tail);
	
/* Add code BEGIN */
    if (!queue_A_initialized){
        printf("NOT INITIALIZED\n");
        return;
    }

    queue_A_node_t *curr = queue_A_head;
	while (curr != NULL) {
        printf("[%p, %p] -> ", curr, curr->item);
        curr = curr->next;
    }

/* Add code END */
	
	printf("(nil)\n");
}

/* Remove any items remaining in the queue and free any dynamically allocated 
 * memory used by the queue for these items, restoring the queue to the fresh, 
 * uninitialized state.
 * Returns 0 on success, or an error if the queue has not been initialized.
 */
int queue_A_destroy()
{

/* Add code BEGIN */

    if (!queue_A_initialized){
        return ERR_NOT_INITIALIZED;
    }

    queue_A_node_t *curr = queue_A_head;
    while (curr != NULL){
        to_be_freed = curr;
        curr = curr->next;
        free(to_be_freed);
    }
    queue_A_head = NULL;
    queue_A_tail = NULL;
    return 0;
/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;	
}

/* END of Part A */

/*************************************************
 * ------------------Part B:---------------------- 
 * In this part, you will implement a 
 * static queue with a max capacity using
 * the RING BUFFER introduced in the handout. 
 * DO NOT add any global (static) variables.
 *
 *************************************************/

bool queue_B_initialized = 0;

typedef struct queue_B_node {

/* Add code BEGIN */

	
/* Add code END */

	void *item;

} queue_B_node_t;

queue_B_node_t queue_B_nodes[PART_B_MAX_SIZE];

/* You may change the names and types of these 3 global variables.
 * You may not add any additional global variables.
 */  
unsigned long queue_B_var_1 = 0;
unsigned long queue_B_var_2 = 0;
unsigned long queue_B_var_3 = 0;

/* Add code BEGIN */

/* Helper functions and macros only! */

/* Add code END */

/* Perform any initialization needed so that the queue data structure can be 
 * used. 
 * Returns 0 on success or an error if the queue has already been initialized.
 */
int queue_B_initialize()
{

/* Add code BEGIN */

	
/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Add item to the tail of the queue.
 * Returns 0 on success or an appropriate error code on failure.
 */
int queue_B_enqueue(void* item)
{

/* Add code BEGIN */


/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Remove the item at the head of the queue and store it in the location
 * pointed to by 'item'.
 * Returns 0 on success or an appropriate error code on failure.
 */
int queue_B_dequeue(void** item)
{

/* Add code BEGIN */

	
/* Add code END */
	
	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Search the queue for 'item' and, if found, remove it from the queue.
 * Returns 0 if the item is found, or an error code if the item is not 
 * in the queue. 
 */
int queue_B_remove_from_queue(void *item)
{
	
/* Add code BEGIN */


/* Add code END */

	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* Print the contents of the queue from head to tail.
 * Uncomment the first printf, and modify it to print the address of the 
 * head and tail of the queue. 
 * Refer to tutorial handout for expected output format. 
 */
void queue_B_print_queue()
{
	/* printf("head: %p, tail: %p\n", <addr of head>, <addr of tail>); */

	/* Add code BEGIN */

	
	/* Add code END */
	
	printf("(nil)\n");
}

/* Remove any items remaining in the queue, restoring the queue to the fresh, 
 * uninitialized state.
 * Returns 0 on success, or an error if the queue has not been initialized.
 */
int queue_B_destroy()
{

/* Add code BEGIN */


/* Add code END */
	
	/* Change the return value when you implement this function. */
	return ERR_NOT_IMPLEMENTED;
}

/* END of Part B */
