#include "sim.h"
#include "coremap.h"

list_head *lru_queue; // Am queue
list_head *fifo_queue; // A1 queue
int fifo_size;
int fifo_threshold;

// Gets the frame of a framelist entry
struct frame *frame_of(list_entry *entry){
    return container_of(entry, struct frame, framelist_entry);
}

/* Page to evict is chosen using the simplified 2Q algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int s2q_evict(void)
{
    list_entry *entry;
    struct frame *to_evict;
    if (fifo_size > fifo_threshold){
        entry = list_first_entry(fifo_queue);
        fifo_size --;
    } else {
        entry = list_first_entry(lru_queue);
    }
    to_evict = frame_of(entry);
    list_del(entry);
	return (to_evict - coremap) / sizeof(frame);
}

/* This function is called on each access to a page to update any information
 * needed by the simplified 2Q algorithm.
 * Input: The page table entry and full virtual address (not just VPN)
 * for the page that is being accessed.
 */
void s2q_ref(int frame, vaddr_t vaddr)
{
    if (coremap[frame].framelist_entry.next == NULL){ // It is not part of a queue yet
        list_add_tail(fifo_queue, coremap[frame].framelist_entry);
        fifo_size ++;
    } else if (get_referenced(coremap[frame].pte)) { // It has been referenced and moved to Am LRU queue
        list_del(coremap[frame].framelist_entry);
        list_add_tail(lru_queue, coremap[frame].framelist_entry);
    } else { // Not yet referenced, should be moved from FIFO to LRU queue
        list_del(coremap[frame].framelist_entry);
        list_add_tail(lru_queue, coremap[frame].framelist_entry);
        fifo_size -= 1;
        set_referenced(coremap[frame].pte, 1);
    }

	(void)vaddr; // To keep the compiler from crying
}

/* Initialize any data structures needed for this replacement algorithm. */
void s2q_init(void)
{
    list_init(lru_queue);
    list_init(fifo_queue);
    fifo_size = 0;
    fifo_threshold = memsize / 10;
}

/* Cleanup any data structures created in s2q_init(). */
void s2q_cleanup(void)
{
    // I mean, technically this does nothing, but whatever
    list_destroy(lru_queue);
    list_destroy(fifo_queue);
}
