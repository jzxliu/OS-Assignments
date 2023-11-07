#include "sim.h"
#include "coremap.h"

/* Page to evict is chosen using the simplified 2Q algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int s2q_evict(void)
{
	assert(false);
	return -1;
}

/* This function is called on each access to a page to update any information
 * needed by the simplified 2Q algorithm.
 * Input: The page table entry and full virtual address (not just VPN)
 * for the page that is being accessed.
 */
void s2q_ref(int frame, vaddr_t vaddr)
{
	(void)frame;
	(void)vaddr;
}

/* Initialize any data structures needed for this replacement algorithm. */
void s2q_init(void)
{

}

/* Cleanup any data structures created in s2q_init(). */
void s2q_cleanup(void)
{

}
