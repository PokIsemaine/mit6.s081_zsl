// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem [NCPU];

// NCPU = 8 , maximum number of CPUs

void
kinit()
{
  char lock_name[11];
  for(int i = 0; i < NCPU; ++i) {
    snprintf(lock_name, sizeof(lock_name), "kmem_cpu_%d", i);
    initlock(&kmem[i].lock, lock_name);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off(); // 关中断 for cpuid() xv6 手册 7.4
  int cpu = cpuid();
  pop_off();  // 开中断

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  
   
  push_off(); // 关中断 for cpuid() xv6 手册 7.4
  int cpu = cpuid();
  pop_off();  // 开中断


  acquire(&kmem[cpu].lock);

  r = kmem[cpu].freelist;
  if(r) {
    kmem[cpu].freelist = r->next;
  }
  release(&kmem[cpu].lock);

  if(!r) {
    // 当前 CPU 空闲内存耗尽，从其他 CPU 窃取
    for(int i = 0; i < NCPU; ++i) {
      // if(i != cpu && kmem[i].freelist) { kmem[i].freelist 这个可能有竞态条件，获取锁后再判断
      if(i != cpu) {

        acquire(&kmem[i].lock);
        r = kmem[i].freelist;
        if(r) {
          kmem[i].freelist = r->next;
          release(&kmem[i].lock);
          break;
        }
        release(&kmem[i].lock);
      }
    }
  }


  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
