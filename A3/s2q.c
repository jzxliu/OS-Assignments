#include "sim.h"
#include "coremap.h"

list_head lru_queue; // Am queue
list_head fifo_queue; // A1 queue
unsigned long fifo_size;
unsigned long fifo_threshold;


/* Page to evict is chosen using the simplified 2Q algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int s2q_evict(void)
{
    list_entry *entry;
    if (fifo_size >= fifo_threshold){
        entry = list_first_entry(&fifo_queue);
        fifo_size -= 1;
    } else {
        entry = list_first_entry(&lru_queue);
    }
    struct frame *to_evict = container_of(entry, struct frame, framelist_entry);
    list_del(entry);
    set_referenced(to_evict->pte, 0);
	return (to_evict - coremap) / sizeof(struct frame);
}

/* This function is called on each access to a page to update any information
 * needed by the simplified 2Q algorithm.
 * Input: The page table entry and full virtual address (not just VPN)
 * for the page that is being accessed.
 */
void s2q_ref(int frame, vaddr_t vaddr)
{
    if (!(list_entry_is_linked(&coremap[frame].framelist_entry))){ // It is not part of a queue yet
        list_add_tail(&fifo_queue, &coremap[frame].framelist_entry);
        set_referenced(coremap[frame].pte, 0);
        fifo_size ++;
    } else if (get_referenced(coremap[frame].pte)) { // It has been referenced and moved to Am LRU queue
        list_del(&coremap[frame].framelist_entry);
        list_add_tail(&lru_queue, &coremap[frame].framelist_entry);
    } else { // Not yet referenced, should be moved from FIFO to LRU queue
        list_del(&coremap[frame].framelist_entry);
        list_add_tail(&lru_queue, &coremap[frame].framelist_entry);
        set_referenced(coremap[frame].pte, 1);
        fifo_size --;
    }

	(void)vaddr; // To keep the compiler from crying
}

/* Initialize any data structures needed for this replacement algorithm. */
void s2q_init(void)
{
    list_init(&lru_queue);
    list_init(&fifo_queue);
    fifo_size = 0;
    fifo_threshold = memsize / 10;
}

/* Cleanup any data structures created in s2q_init(). */
void s2q_cleanup(void)
{
    // I mean, technically this does nothing, but whatever
    list_destroy(&lru_queue);
    list_destroy(&fifo_queue);
}
