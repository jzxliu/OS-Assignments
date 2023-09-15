#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/*************************************************
 * -------------------CSC369----------------------
 * ------------------T1 Defines--------------------- 
 *************************************************/

#define malloc malloc_t1
#define free free_t1
#define calloc(n,s) malloc(n*s)


/* ERROR CODES */
#define ERR_FULL            -1  /* The maximum capacity is reached. */
#define ERR_INVALID_ARG     -2  /* Invalid pointer is passed to dequeue. */
#define ERR_EMPTY           -3  /* The queue is empty when dequeue is called. */
#define ERR_NO_SUCH_ITEM    -4  /* The item to be removed is not in the queue. */ 
#define ERR_NOT_INITIALIZED -5  /* Initialization function is not called yet. */
#define ERR_INITIALIZED     -6  /* Try to initialize when already initialized. */
#define ERR_NOT_IMPLEMENTED -9  /* function not implemented yet */

#define PART_B_MAX_SIZE 1024

void * malloc_t1(size_t size); 
void free_t1(void * ptr);

