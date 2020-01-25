/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_small(unsigned long size)
{
    unsigned long taille;
    if (arena.chunkpool == NULL){
        taille = mem_realloc_small();

        void *cur = arena.chunkpool;

        // Construction de la liste chainée
        for (unsigned long i = 0 ; i < taille ; i += CHUNKSIZE){
            unsigned long addr = ((unsigned long)(cur))+i;
            *(unsigned long *)(((unsigned long)cur)+i) = addr;
        }

        // Dernier élément
        *(unsigned long *)(((unsigned long)cur)+taille) = (unsigned long) NULL;
    }
    // "Enlève" le premier élément
    unsigned long *ptr = arena.chunkpool;
    ptr += CHUNKSIZE; 
    

    MemKind k = SMALL_KIND;
    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, k);
}

void efree_small(Alloc a) {
    unsigned long addr = (unsigned long) a.ptr;
    *(unsigned long *)(((unsigned long)a.ptr)-CHUNKSIZE) = addr;
    arena.chunkpool = (unsigned long *)((a.ptr)-CHUNKSIZE);
}
