// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef _POOLALLOC_INCLUDED_
#define _POOLALLOC_INCLUDED_

//
// This header defines an allocator that can be used to efficiently
// allocate a large number of small requests for heap memory, with the
// intention that they are not individually deallocated, but rather
// collectively deallocated at one time.
//
// This simultaneously
//
// * Makes each individual allocation much more efficient; the
//     typical allocation is trivial.
// * Completely avoids the cost of doing individual deallocation.
// * Saves the trouble of tracking down and plugging a large class of leaks.
//
// Individual classes can use this allocator by supplying their own
// new and delete methods.
//
// STL containers can use this allocator by using the pool_allocator
// class as the allocator (second) template argument.
//

#include <stddef.h>
#include <vector>


// Set the allocation functions used to initialize our global allocators.
typedef void*(*GlobalAllocateFunction)(unsigned, void*);
typedef void(*GlobalFreeFunction)(void*, void*);
void SetGlobalAllocationAllocator(GlobalAllocateFunction alloc, GlobalFreeFunction free, void* user);

class TAllocation
{
public:
   TAllocation(size_t size, unsigned char* mem, TAllocation* prev = 0) :
      size(size), mem(mem), prevAlloc(prev)
   {
   }

private:
   unsigned char* data()      const { return mem; }

   size_t size;                  // size of the user data area
   unsigned char* mem;           // beginning of our allocation (pts to header)
   TAllocation* prevAlloc;       // prior allocation in the chain
};

//
// There are several stacks.  One is to track the pushing and popping
// of the user, and not yet implemented.  The others are simply a
// repositories of free pages or used pages.
//
// Page stacks are linked together with a simple header at the beginning
// of each allocation obtained from the underlying OS.  Multi-page allocations
// are returned to the OS.  Individual page allocations are kept for future
// re-use.
//
// The "page size" used is not, nor must it match, the underlying OS
// page size.  But, having it be about that size or equal to a set of
// pages is likely most optimal.
//
class TPoolAllocator {
public:
   TPoolAllocator(bool global = false, int growthIncrement = 8*1024, int allocationAlignment = 16);

   //
   // Don't call the destructor just to free up the memory, call pop()
   //
   ~TPoolAllocator();

   //
   // Call push() to establish a new place to pop memory too.  Does not
   // have to be called to get things started.
   //
   void push();

   //
   // Call pop() to free all memory allocated since the last call to push(),
   // or if no last call to push, frees all memory since first allocation.
   //
   void pop();

   //
   // Call popAll() to free all memory allocated.
   //
   void popAll();

   //
   // Call allocate() to actually acquire memory.  Returns 0 if no memory
   // available, otherwise a properly aligned pointer to 'numBytes' of memory.
   //
   void* allocate(size_t numBytes);

   //
   // There is no deallocate.  The point of this class is that
   // deallocation can be skipped by the user of it, as the model
   // of use is to simultaneously deallocate everything at once
   // by calling pop(), and to not have to solve memory leak problems.
   //

protected:
   friend struct tHeader;

   struct tHeader
   {
      tHeader(tHeader* nextPage, size_t pageCount) :
         nextPage(nextPage), pageCount(pageCount)
      {
      }

      ~tHeader()
      {
      }

      tHeader* nextPage;
      size_t pageCount;
   };

   struct tAllocState
   {
      size_t offset;
      tHeader* page;
   };
   typedef std::vector<tAllocState> tAllocStack;

   bool global;            // should be true if this object is globally scoped
   size_t pageSize;        // granularity of allocation from the OS
   size_t alignment;       // all returned allocations will be aligned at
                         //      this granularity, which will be a power of 2
   size_t alignmentMask;
   size_t headerSkip;      // amount of memory to skip to make room for the
                         //      header (basically, size of header, rounded
                         //      up to make it aligned
   size_t currentPageOffset;  // next offset in top of inUseList to allocate from
   tHeader* freeList;      // list of popped memory
   tHeader* inUseList;     // list of all memory currently being used
   tAllocStack stack;      // stack of where to allocate from, to partition pool

   int numCalls;           // just an interesting statistic
   size_t totalBytes;      // just an interesting statistic
private:
   TPoolAllocator& operator=(const TPoolAllocator&);  // dont allow assignment operator
   TPoolAllocator(const TPoolAllocator&);  // dont allow default copy constructor
};


//
// There could potentially be many pools with pops happening at
// different times.  But a simple use is to have a global pop
// with everyone using the same global allocator.
//
typedef TPoolAllocator* PoolAllocatorPointer;
extern TPoolAllocator& GetGlobalPoolAllocator();
#define GlobalPoolAllocator GetGlobalPoolAllocator()


struct TThreadGlobalPools
{
   TPoolAllocator* globalPoolAllocator;
};

void SetGlobalPoolAllocatorPtr(TPoolAllocator* poolAllocator);

//
// This STL compatible allocator is intended to be used as the allocator
// parameter to templatized STL containers, like vector and map.
//
// It will use the pools for allocation, and not
// do any deallocation, but will still do destruction.
//
template<class T>
class pool_allocator
{
public:
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef T *pointer;
   typedef const T *const_pointer;
   typedef T& reference;
   typedef const T& const_reference;
   typedef T value_type;
   template<class Other>
      struct rebind
      {
         typedef pool_allocator<Other> other;
      };
   pointer address(reference x) const { return &x; }
   const_pointer address(const_reference x) const { return &x; }

#ifdef USING_SGI_STL
   pool_allocator()  { }
#else
   pool_allocator() : allocator(&GlobalPoolAllocator) { }
   pool_allocator(TPoolAllocator& a) : allocator(&a) { }
   pool_allocator(const pool_allocator<T>& p) : allocator(p.allocator) { }
   pool_allocator& operator= (const pool_allocator<T>& rhs)
   {
      allocator = rhs.allocator;
      return *this;
   }
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1300
   template<class Other>
#ifdef USING_SGI_STL
      pool_allocator(const pool_allocator<Other>& p) /*: allocator(p.getAllocator())*/ { }
#else
      pool_allocator(const pool_allocator<Other>& p) : allocator(&p.getAllocator()) { }
#endif
#endif

#ifndef _WIN32
      template<class Other>
         pool_allocator(const pool_allocator<Other>& p) : allocator(&p.getAllocator()) { }
#endif

#ifdef USING_SGI_STL
   static pointer allocate(size_type n)
   {
      return reinterpret_cast<pointer>(getAllocator().allocate(n));
   }
   pointer allocate(size_type n, const void*)
   {
      return reinterpret_cast<pointer>(getAllocator().allocate(n));
   }

   static void deallocate(void*, size_type) { }
   static void deallocate(pointer, size_type) { }
#else
   pointer allocate(size_type n)
   {
      return reinterpret_cast<pointer>(getAllocator().allocate(n * sizeof(T)));
   }
   pointer allocate(size_type n, const void*)
   {
      return reinterpret_cast<pointer>(getAllocator().allocate(n * sizeof(T)));
   }

   void deallocate(void*, size_type) { }
   void deallocate(pointer, size_type) { }
#endif

   pointer _Charalloc(size_t n)
   {
      return reinterpret_cast<pointer>(getAllocator().allocate(n));
   }

   void construct(pointer p, const T& val) { new ((void *)p) T(val); }
   void destroy(pointer p) { p->T::~T(); }

   bool operator==(const pool_allocator& rhs) const { return &getAllocator() == &rhs.getAllocator(); }
   bool operator!=(const pool_allocator& rhs) const { return &getAllocator() != &rhs.getAllocator(); }

   size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T); }
   size_type max_size(int size) const { return static_cast<size_type>(-1) / size; }

#ifdef USING_SGI_STL
   //void setAllocator(TPoolAllocator* a) { allocator = a; }
   static  TPoolAllocator& getAllocator() { return GlobalPoolAllocator; }
#else
   void setAllocator(TPoolAllocator* a) { allocator = a; }
   TPoolAllocator& getAllocator() const { return *allocator; }

protected:
   TPoolAllocator* allocator;
#endif
};

#endif // _POOLALLOC_INCLUDED_
