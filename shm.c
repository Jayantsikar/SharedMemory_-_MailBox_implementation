// kernel/shm.c
#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "memlayout.h"
#include "proc.h"
#include "defs.h"

struct shm_region {
  int   used;
  int   key;
  char *pa;        // physical page
  int   refcnt;    // # attached processes
};

// Global SHM table
static struct {
  struct spinlock lock;
  struct shm_region reg[NSHM];
  int inited;
} shm;

static void
shm_init(void)
{
  if(!shm.inited){
    initlock(&shm.lock, "shm_tbl");
    for(int i=0;i<NSHM;i++){
      shm.reg[i].used = 0;
      shm.reg[i].key = 0;
      shm.reg[i].pa = 0;
      shm.reg[i].refcnt = 0;
    }
    shm.inited = 1;
  }
}

// Map each slot i to a high VA window below TRAPFRAME.
// (Fixed VA per slot across all processes.)
static inline uint64
slot_va(int idx)
{
  return TRAPFRAME - ((uint64)(idx + 1) * PGSIZE);
}

static inline int
mapped_at(pagetable_t pt, uint64 va)
{
  return walkaddr(pt, va) != 0;
}

// find slot by key; return index or -1
static int
find_slot_nolock(int key)
{
  for(int i=0;i<NSHM;i++)
    if(shm.reg[i].used && shm.reg[i].key == key)
      return i;
  return -1;
}

// find free slot; return index or -1
static int
find_free_nolock(void)
{
  for(int i=0;i<NSHM;i++)
    if(!shm.reg[i].used)
      return i;
  return -1;
}

int
shm_create_k(int key)
{
  shm_init();
  acquire(&shm.lock);
  int idx = find_slot_nolock(key);
  if(idx >= 0){
    release(&shm.lock);
    return idx; // already exists
  }
  idx = find_free_nolock();
  if(idx < 0){
    release(&shm.lock);
    return -1;
  }

  char *pa = kalloc();
  if(pa == 0){
    release(&shm.lock);
    return -1;
  }
  memset(pa, 0, PGSIZE);

  shm.reg[idx].used   = 1;
  shm.reg[idx].key    = key;
  shm.reg[idx].pa     = pa;
  shm.reg[idx].refcnt = 0;

  release(&shm.lock);
  return idx;
}

// Returns VA on success, 0 on failure.
uint64
shm_get_k(int key)
{
  shm_init();
  struct proc *p = myproc();

  acquire(&shm.lock);
  int idx = find_slot_nolock(key);
  if(idx < 0){
    release(&shm.lock);
    return 0; // region not created
  }
  uint64 va = slot_va(idx);
  // Fast path: already mapped for this proc.
  if(mapped_at(p->pagetable, va)){
    release(&shm.lock);
    return va;
  }

  // Map and bump refcnt atomically under the table lock to avoid
  // transient negative/over/under counts on concurrent attachers.
  if(mappages(p->pagetable, va, PGSIZE, (uint64)shm.reg[idx].pa, PTE_U|PTE_R|PTE_W) < 0){
    release(&shm.lock);
    return 0;
  }
  shm.reg[idx].refcnt++;
  release(&shm.lock);
  return va;
}

// Detach from current process; free physical page when last detaches.
int
shm_close_k(int key)
{
  shm_init();
  struct proc *p = myproc();

  acquire(&shm.lock);
  int idx = find_slot_nolock(key);
  if(idx < 0){
    release(&shm.lock);
    return -1;
  }
  uint64 va = slot_va(idx);

  // Only detach/count-down if this proc actually had it mapped.
  int was_mapped = mapped_at(p->pagetable, va);
  release(&shm.lock);

  if(was_mapped){
    // Unmap outside the shm.lock; do_free=0 since it's shared.
    uvmunmap(p->pagetable, va, 1, 0);

    // Now adjust refcount and possibly free the region.
    acquire(&shm.lock);
    if(shm.reg[idx].refcnt > 0)
      shm.reg[idx].refcnt--;
    int freeit = (shm.reg[idx].refcnt == 0 && shm.reg[idx].pa != 0);
    char *pa = shm.reg[idx].pa;
    if(freeit){
      shm.reg[idx].used = 0;
      shm.reg[idx].key  = 0;
      shm.reg[idx].pa   = 0;
    }
    release(&shm.lock);
    if(freeit)
      kfree(pa);
  }
  return 0;
}

// --- Auto-detach to prevent freewalk panics ---

// Detach all SHM mappings from p's CURRENT pagetable (called in exit()).
void
shm_on_exit_all(struct proc *p)
{
  shm_init();
  if(p == 0 || p->pagetable == 0) return;

  for(int i=0;i<NSHM;i++){
    uint64 va = slot_va(i);
    if(mapped_at(p->pagetable, va)){
      uvmunmap(p->pagetable, va, 1, 0); // shared: do_free=0

      acquire(&shm.lock);
      if(shm.reg[i].refcnt > 0)
        shm.reg[i].refcnt--;
      int freeit = (shm.reg[i].refcnt == 0 && shm.reg[i].pa != 0);
      char *pa = shm.reg[i].pa;
      if(freeit){
        shm.reg[i].used = 0;
        shm.reg[i].key  = 0;
        shm.reg[i].pa   = 0;
      }
      release(&shm.lock);
      if(freeit)
        kfree(pa);
    }
  }
}

// Detach from the OLD pagetable during exec() (before freeing it).
void
shm_on_exec_all(pagetable_t old, struct proc *p)
{
  shm_init();
  if(old == 0) return;

  for(int i=0;i<NSHM;i++){
    uint64 va = slot_va(i);
    if(mapped_at(old, va)){
      uvmunmap(old, va, 1, 0);

      acquire(&shm.lock);
      if(shm.reg[i].refcnt > 0)
        shm.reg[i].refcnt--;
      int freeit = (shm.reg[i].refcnt == 0 && shm.reg[i].pa != 0);
      char *pa = shm.reg[i].pa;
      if(freeit){
        shm.reg[i].used = 0;
        shm.reg[i].key  = 0;
        shm.reg[i].pa   = 0;
      }
      release(&shm.lock);
      if(freeit)
        kfree(pa);
    }
  }
}
