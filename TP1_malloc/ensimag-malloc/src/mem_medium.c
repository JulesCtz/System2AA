/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}


void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);

    unsigned int blocInd = puiss2(size);

    if (arena.TZL[blocInd] == NULL) {
        decoupageBloc(blocInd+1);
    }

    unsigned long * ptr = (unsigned long *) arena.TZL[blocInd];

    return mark_memarea_and_get_user_ptr(ptr, size+32, MEDIUM_KIND);
    //return (void *) 0;
}

void decoupageBloc(unsigned int blocInd){
    if(blocInd == FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {
        mem_realloc_medium();
    }
    if(arena.TZL[blocInd] == NULL){
        decoupageBloc(blocInd+1);
    }
    unsigned long *suiv = (unsigned long *)arena.TZL[blocInd];

    unsigned long blocSize = pow(2, blocInd-1);

    void ** ptr = (void *)arena.TZL[blocInd];
    unsigned long nextAddr = *(unsigned long *)ptr ^ blocSize;


    arena.TZL[blocInd-1] = ptr;

    void *nextPtr = (void **) nextAddr;
    *ptr = nextPtr;

    nextPtr = NULL;

    arena.TZL[blocInd] = suiv;
}


void efree_medium(Alloc a) {
    /*unsigned long addr = (unsigned long) a.ptr;
    unsigned long size = a.size;
    unsigned long addrBuddy = size ^ addr;
    unsigned int blocInd = puiss2(size);
    void ** cur = (void **) arena.TZL[blocInd];
    void ** ptr = (void **) addr;
    while(*cur != NULL){
        if(*cur == (void *)addrBuddy){
            *ptr = (void *)addrBuddy;
            void ** ptrBuddy = (void **) addrBuddy;
            *ptrBuddy = *(void **)arena.TZL[blocInd+1];
            arena.TZL[blocInd+1] = *ptr;
            return;
        }
    }
    *ptr = *(void **)arena.TZL[blocInd];
    arena.TZL[blocInd] = *ptr;
    */
}


