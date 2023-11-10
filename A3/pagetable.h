#ifndef __PAGETABLE_H__
#define __PAGETABLE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define NUM_ENTRIES 512 // 2^9


// User-level virtual addresses on a 64-bit Linux system are 48 bits in our
// traces, and the page size is 4096 (12 bits). The remaining 36 bits are
// the virtual page number, which is used as the lookup key (or index) into
// your page table.

// Page table entry 
// This structure will need to record the physical page frame number
// for a virtual page, as well as the swap offset if it is evicted. 
// You will also need to keep track of the Valid, Dirty and Referenced
// status bits (or flags). 
// You do not need to keep track of Read/Write/Execute permissions.
typedef struct pt_entry_s {
    unsigned int frame_number;
    off_t swap_offset;
    bool valid : 1;
    bool dirty : 1;
    bool referenced : 1;
} pt_entry_t;

// I decided to use a 4 level page table design with 9 bits to index into per level.
// So we will have 4 layers with 2^9 = 512 entries.

// Bottom Level Page Table
typedef struct pt_bottom_s {
    pt_entry_t* entries[NUM_ENTRIES];
} pt_bottom_t;

// Middle Level Page Table
typedef struct pt_middle_s {
    pt_bottom_t* entries[NUM_ENTRIES];
} pt_middle_t;

// Top Level Page Table
typedef struct pt_top_s {
    pt_middle_t* entries[NUM_ENTRIES];
} pt_top_t;

// Page Directory
typedef struct pt_directory_s {
    pt_top_t* entries[NUM_ENTRIES];
} pt_directory_t;

#endif /* __PAGETABLE_H__ */
