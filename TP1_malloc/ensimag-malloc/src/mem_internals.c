/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    unsigned long magic = ((knuth_mmix_one_round((unsigned long) ptr) >> 2) <<2) | k;
    *(unsigned long*)ptr = size;
    *(unsigned long*)(ptr+8) = magic;
    *(unsigned long*)(ptr+size-16) = magic;
    *(unsigned long*)(ptr+size-8) = size;

    //printf("%lu\n", magic);
    return ptr+16;
}




    Alloc
mark_check_and_get_alloc(void *ptr)
{
    void* begin = (ptr-16);

    MemKind k = *(unsigned long *)(begin+8) & 0b11UL;

    unsigned long magic = ((knuth_mmix_one_round((unsigned long)begin) >> 2) <<2) | k;
    assert(magic == *(unsigned long *)(begin+8));
    unsigned long total_size = *(unsigned long *)begin;
    //printf("%lu\n", magic);
    //printf("%lu\n\n\n\n", *(unsigned long*)(ptr-16+total_size-16));

    assert(
            *(unsigned long*)(ptr-16) == *(unsigned long*)(ptr-16+total_size-8) &&
            *(unsigned long*)(ptr-8) == *(unsigned long*)(ptr-16+total_size-16)
          );
    Alloc a = {ptr-16,k,total_size};
    return a;
}



unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
