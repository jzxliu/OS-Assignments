/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Andrew Peterson, Karen Reid, Alexey Khrabrov, Angela Brown, Kuei Sun
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019, 2021 Karen Reid
 * Copyright (c) 2023, Angela Brown, Kuei Sun
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "malloc369.h"
#include "sim.h"
#include "coremap.h"
#include "swap.h"
#include "pagetable.h"


// Counters for various events.
// Your code must increment these when the related events occur.
size_t hit_count = 0;
size_t miss_count = 0;
size_t ref_count = 0;
size_t evict_clean_count = 0;
size_t evict_dirty_count = 0;

// Define a Page Directory
pt_directory_t *page_directory;

// Accessor functions for page table entries, to allow replacement
// algorithms to obtain information from a PTE, without depending
// on the internal implementation of the structure.

/* Returns true if the pte is marked valid, otherwise false */
bool is_valid(pt_entry_t *pte)
{
	return pte->valid;
}

/* Returns true if the pte is marked dirty, otherwise false */
bool is_dirty(pt_entry_t *pte)
{
	return pte->dirty;
}

/* Returns true if the pte is marked referenced, otherwise false */
bool get_referenced(pt_entry_t *pte)
{
	return pte->referenced;
}

/* Sets the 'referenced' status of the pte to the given val */
void set_referenced(pt_entry_t *pte, bool val)
{
	pte->referenced = val;
}

/*
 * Initializes your page table.
 * This function is called once at the start of the simulation.
 * For the simulation, there is a single "process" whose reference trace is
 * being simulated, so there is just one overall page table.
 *
 * In a real OS, each process would have its own page table, which would
 * need to be allocated and initialized as part of process creation.
 * 
 * The format of the page table, and thus what you need to do to get ready
 * to start translating virtual addresses, is up to you. 
 */
void init_pagetable(void)
{
    page_directory = malloc369(sizeof(pt_directory_t));
    memset(page_directory, 0, sizeof(pt_directory_t));
}

/*
 * Write virtual page represented by pte to swap, if needed, and update 
 * page table entry.
 *
 * Called from allocate_frame() in coremap.c after a victim page frame has
 * been selected. 
 *
 * Counters for evictions should be updated appropriately in this function.
 */
void handle_evict(pt_entry_t * pte)
{
    pte->swap_offset = swap_pageout(pte->frame_number, INVALID_SWAP);
    pte->valid = false;
}

/*
 * Looks up the PTE by indexing using the vaddr.
 * Allocate new page tables & initialize the entry if the entry does not already exist.
 */
pt_entry_t *pagetable_lookup(vaddr_t vaddr){
    // Calculate the indexes for the multi-level page table by shifting and looking at the rightmost 9 bits.
    size_t directory_index = (vaddr >>39) & 0x1FF;
    size_t top_index = (vaddr >> 30) & 0x1FF;
    size_t mid_index = (vaddr >> 21) & 0x1FF;
    size_t bottom_index = (vaddr >> 12) & 0x1FF;

    pt_top_t *top = page_directory->entries[directory_index];
    if (!top) { // Initialize top page table
        page_directory->entries[directory_index] = malloc369(sizeof(pt_top_t));
        top = page_directory->entries[directory_index];
        memset(top, 0, sizeof(pt_top_t));
    }

    pt_middle_t *mid = top->entries[top_index];
    if (!mid) { // Initialize middle page table
        top->entries[top_index] = malloc369(sizeof(pt_middle_t));
        mid = top->entries[top_index];
        memset(mid, 0, sizeof(pt_middle_t));
    }

    pt_bottom_t *bot = mid->entries[mid_index];
    if (!bot) { // Initialize bottom page table
        mid->entries[mid_index] = malloc369(sizeof(pt_bottom_t));
        bot = mid->entries[mid_index];
        memset(bot, 0, sizeof(pt_bottom_t));
    }

    pt_entry_t *entry = bot->entries[bottom_index];
    if (!entry) { // If this is the first time initializing page table entry
        bot->entries[bottom_index] = malloc369(sizeof(pt_entry_t));
        entry = bot->entries[bottom_index];
        entry->valid = 1; // Initialize to not valid
        entry->dirty = 0; // Initially not dirty
        entry->referenced = 0;
        entry->frame_number = allocate_frame(entry);
        init_frame(entry->frame_number);
        entry->swap_offset = INVALID_SWAP;
    }

    return entry;
}


/*
 * Locate the physical frame number for the given vaddr using the page table.
 *
 * If the page table entry is invalid and not on swap, then this is the first 
 * reference to the page and a (simulated) physical frame should be allocated 
 * and initialized to all zeros (using init_frame from coremap.c).
 * If the page table entry is invalid and on swap, then a (simulated) physical 
 * frame should be allocated and filled by reading the page data from swap.
 *
 * Make sure to update page table entry status information:
 *  - the page table entry should be marked valid
 *  - if the type of access is a write ('S'tore or 'M'odify),
 *    the page table entry should be marked dirty
 *  - a page should be marked dirty on the first reference to the page,
 *    even if the type of access is a read ('L'oad or 'I'nstruction type).
 *  - DO NOT UPDATE the page table entry 'referenced' information. That
 *    should be done by the replacement algorithm functions.
 *
 * When you have a valid page table entry, return the page frame number
 * that holds the requested virtual page.
 *
 * Counters for hit, miss and reference events should be incremented in
 * this function.
 */
int find_frame_number(vaddr_t vaddr, char type)
{
    pt_entry_t *entry = pagetable_lookup(vaddr);
    if (!entry->valid) {
        entry->frame_number = allocate_frame(entry);
        swap_pagein(entry->frame_number, entry->swap_offset);
        entry->valid = 1;
    }
    return entry->frame_number;
}

void print_pagetable(void)
{
}


void free_pagetable(void)
{
    // Iterate through the page table at every level and free them from bottom up.
    for (int i = 0; i < NUM_ENTRIES; i++) {
        pt_top_t *top = page_directory->entries[i];
        if (top) {
            for (int j = 0; j < NUM_ENTRIES; j++) {
                pt_middle_t *mid = top->entries[j];
                if (mid) {
                    for (int k = 0; k < NUM_ENTRIES; k++) {
                        pt_bottom_t *bot = mid->entries[k];
                        if (bot) {
                            free369(bot);
                        }
                    }
                    free369(mid);
                }
            }
            free369(top);
        }
    }
    free(page_directory);
}
