// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __MEMORY_H
#define __MEMORY_H

typedef struct MemoryPool_rec MemoryPool;

extern MemoryPool *mem_CreatePool(size_t chunksize, unsigned align);
extern void mem_FreePool(MemoryPool *);
extern void *mem_Alloc(MemoryPool *p, size_t size);


#endif /* __MEMORY_H */
