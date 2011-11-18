// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __STDC99__
#include <stdint.h>
#elif defined (_WIN64)
typedef unsigned __int64 uintptr_t;
#elif defined (_WIN32)
typedef unsigned int uintptr_t;
#else 
typedef unsigned long int uintptr_t;
#endif

#include "memory.h"

// default alignment and chunksize, if called with 0 arguments
#define CHUNKSIZE       (64*1024)
#define ALIGN           8

// we need to call the `real' malloc and free, not our replacements
#undef malloc
#undef free

struct chunk {
    struct chunk        *next;
};

struct MemoryPool_rec {
    struct chunk        *next;
    uintptr_t           free, end;
    size_t              chunksize;
    uintptr_t           alignmask;
};

MemoryPool *mem_CreatePool(size_t chunksize, unsigned int align)
{
    MemoryPool  *pool;

    if (align == 0) align = ALIGN;
    if (chunksize == 0) chunksize = CHUNKSIZE;
    if (align & (align-1)) return 0;
    if (chunksize < sizeof(MemoryPool)) return 0;
    if (chunksize & (align-1)) return 0;
    if (!(pool = malloc(chunksize))) return 0;
    pool->next = 0;
    pool->chunksize = chunksize;
    pool->alignmask = (uintptr_t)(align)-1;  
    pool->free = ((uintptr_t)(pool + 1) + pool->alignmask) & ~pool->alignmask;
    pool->end = (uintptr_t)pool + chunksize;
    return pool;
}

void mem_FreePool(MemoryPool *pool)
{
    struct chunk        *p, *next;

    for (p = (struct chunk *)pool; p; p = next) {
        next = p->next;
        free(p);
    }
}

void *mem_Alloc(MemoryPool *pool, size_t size)
{
    struct chunk *ch;
    void *rv = (void *)pool->free;
    size = (size + pool->alignmask) & ~pool->alignmask;
    if (size <= 0) size = pool->alignmask;
    pool->free += size;
    if (pool->free > pool->end || pool->free < (uintptr_t)rv) {
        size_t minreq = (size + sizeof(struct chunk) + pool->alignmask)
                      & ~pool->alignmask;
        pool->free = (uintptr_t)rv;
        if (minreq >= pool->chunksize) {
            // request size is too big for the chunksize, so allocate it as
            // a single chunk of the right size
            ch = malloc(minreq);
            if (!ch) return 0;
        } else {
            ch = malloc(pool->chunksize);
            if (!ch) return 0;
            pool->free = (uintptr_t)ch + minreq;
            pool->end = (uintptr_t)ch + pool->chunksize;
        }
        ch->next = pool->next;
        pool->next = ch;
        rv = (void *)(((uintptr_t)(ch+1) + pool->alignmask) & ~pool->alignmask);
    }
    return rv;
}
