#include "defines.h"

/*************************************************
 * -------------------CSC369----------------------
 * ------------------T1 --------------------- 
 * Dequeue functions: 
 *      If success, store the item into 
 *      the *item* pointer and return 0. 
 *      If failure, return ERROR CODE.
 * Enqueue functions:
 *      If success, return 0.
 *      If failure, return ERROR CODE.
 * Remove functions:
 *      If success return 0
 *      If failure, return ERROR CODE.
 * Initialize functions:
 *      If success, return 0.
 *      If failure, return ERROR CODE.
 * Destroy functions:
 *      If success, return 0.
 *      If failure, return ERROR CODE.
 * 
 * Find the ERROR CODE in defines.h
 *************************************************/

int queue_A_initialize();
int queue_A_enqueue(void *item);
int queue_A_dequeue(void **item);
int queue_A_remove_from_queue(void *item);
void queue_A_print_queue();
int queue_A_destroy();


int queue_B_initialize();
int queue_B_enqueue(void *item);
int queue_B_dequeue(void **item);
int queue_B_remove_from_queue(void *item);
void queue_B_print_queue();
int queue_B_destroy();


