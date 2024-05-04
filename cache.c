// Created by Harshil Varia on 2024-04-5.
// The assignment is a implementation of caching,
// I have used Fully associative cache as it was leveraging runtime with more hits, which was one of the parts of grading this assignment
// There 2 basic functions of cache which is it checks if it is caching the line required if not(miss) asks a level below for the required block.

#include <stddef.h>
#include "cache.h"

#define BLOCK_SIZE 128

// using fully associative cache i.e 1 set with all cache lines
typedef struct {
    unsigned int valid; // checks if the cache line stores a block
    unsigned int tag; // to identify the required block
    char data[BLOCK_SIZE]; //
    unsigned int counter; // Counter to track line usage
} CacheLine;

typedef struct {
    int initialized; // represents if the cache has been initialized or not.
    unsigned int lines; // number of lines in cache
    unsigned int line_size; // size of block in lines
    CacheLine *sets[0];
} Cache;

static void init(void *cache) {
    // Initialize cache sets and lines
    Cache* memory= (Cache*) cache;
    memory->lines= c_info.F_size/BLOCK_SIZE; // as in Fully associative cache E=C/B
    memory->line_size= (c_info.F_size/memory->lines);
    memory->initialized= 1;
    for (unsigned int i = 0; i <memory->lines; i++) {
        memory->sets[i] = (CacheLine *)(&memory->sets[memory->lines]); // For each line in allocating memory of the size CacheLine
        memory->sets[i]->valid=0;// For each line in cache initializing valid bit.
    }
}


extern int cache_get(unsigned long address, unsigned long *value) {

    Cache* cache = (Cache*) c_info.F_memory; // points cache to the start of the F_memory

    if (!cache[0].initialized) {
        // if the cache is not initialized then set up all structures and initialized flag to 1.
        init(c_info.F_memory);
    }
    unsigned int tag = address >> 6; // Tag is the high-order bits after removing the 6-bit block offset
    unsigned int offset = address & 0b111111; // 6 bits for block offset within the 64-byte line
    unsigned int index = 0;

    CacheLine *line = &cache[0].sets[index][0];

    int victim_line=0;
    unsigned int min_counter = line[0].counter;
    // Check for a cache hit
    for (size_t i = 0; i < cache->lines; i++) {
        if (line[i].valid && line[i].tag == tag) {
            *value = *((unsigned long *)(line[i].data+offset));
            line[i].counter++;// increment the counter each time the line is used/accessed
            return 1;
        }else if (line[i].counter < min_counter && !line[i].valid) { // See if we have any empty line in the set
            min_counter = line[i].counter;
            victim_line = i;
        }
    }

    // request memory to give us the block with requested address and store in our line's block
    if (memget(address-offset, &line[victim_line].data, cache->line_size)) {
        *value = *((unsigned long *)(line[victim_line].data+offset));
        // update all the related data of our block.
        line[victim_line].valid = 1;
        line[victim_line].tag = tag;
        line[victim_line].counter=1;
        // Load the block from main memory
        return 1;
    } else {
        return 0;
    }
}

