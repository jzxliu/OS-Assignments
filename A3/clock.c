#include "sim.h"
#include "coremap.h"

static int clock_hand;

/* Page to evict is chosen using the CLOCK algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int clock_evict(void)
{
    while (true) {
        int previous = clock_hand;
        clock_hand = (clock_hand + 1) % memsize;
        if (get_referenced(coremap[previous].pte)){
            set_referenced(coremap[previous].pte, 0);
        } else {
            return previous;
        }
    }
}

/* This function is called on each access to a page to update any information
 * needed by the CLOCK algorithm.
 * Input: The page table entry and full virtual address (not just VPN)
 * for the page that is being accessed.
 */
void clock_ref(int frame, vaddr_t vaddr)
{
    (void)vaddr;
    set_referenced(coremap[frame].pte, 1);
}

/* Initialize any data structures needed for this replacement algorithm. */
void clock_init(void)
{
    clock_hand = 0;
}

/* Cleanup any data structures created in clock_init(). */
void clock_cleanup(void)
{

}
