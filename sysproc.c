#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getyear(void){
  return 1975;
}

extern struct proc proc[];

uint64
sys_top(void)
{
  struct proc *p;

  printf("PID\tState\t\tName\t\tTicks\n");

  for(p = proc; p < &proc[NPROC]; p++) {//proc is array of processes
    
    acquire(&p->lock);
    if(p->state != UNUSED) {
      printf("%d        %s         %s\n", p->pid, 
             p->state == RUNNING ? "RUNNING" :
             p->state == SLEEPING ? "SLEEPING" :
             p->state == ZOMBIE ? "ZOMBIE" :
             p->state == RUNNABLE ? "RUNNABLE" :
             "OTHER",
             p->name);  // <--- Here
    }
    release(&p->lock);
  }

  return 0;
}

// In kernel/sysproc.c

uint64
sys_set_priority(void)
{
  int n = 0; // Initialize to a known value
  struct proc *p = myproc();

  // For your version of xv6, call argint without checking its return value.
  argint(0, &n);

  // You can still use a printf here to see what value argint retrieved.
  // This is the key to debugging your original problem!
  printf("kernel debug: pid %d received priority value %d\n", p->pid, n);
  
  if (n <= 0 || n > MAX_PRIORITY) { // validate the retrieved value
    return (uint64)-1;
  }

  acquire(&p->lock);
  p->priority = n;
  release(&p->lock);
  
  return 0;
}

uint64
sys_get_priority(void)
{
  struct proc *p = myproc();
  int pr;

  acquire(&p->lock);
  pr = p->priority;
  release(&p->lock);

  return (uint64)pr;
}

uint64
sys_get_rtime(void)
{
  // myproc() returns a pointer to the current process's struct proc.
  return myproc()->rtime; 
}

// kernel/sysproc.c
uint64
sys_setburst(void)
{
  int n;
  argint(0, &n);
  if(n < 0)
    return -1;
  if(n <= 0)
    return -1;

  struct proc *p = myproc();
  acquire(&p->lock);
  p->est_burst = n;
  p->remaining = n;
  release(&p->lock);

  return 0;
}

//changes for task 1 

uint64
sys_shm_create(void)
{
  int key; argint(0, &key);
  return shm_create_k(key);
}

uint64
sys_shm_get(void)
{
  int key; argint(0, &key);
  return shm_get_k(key);        // returns VA (0 on error)
}

uint64
sys_shm_close(void)
{
  int key; argint(0, &key);
  return shm_close_k(key);
}

uint64
sys_mbox_create(void)
{
  int key; argint(0, &key);
  return mbox_create_k(key);
}

uint64
sys_mbox_send(void)
{
  int id, msg;
  argint(0, &id);
  argint(1, &msg);
  return mbox_send_k(id, msg);
}

uint64
sys_mbox_recv(void)
{
  int id; uint64 uptr;
  argint(0, &id);
  argaddr(1, &uptr);
  int v;
  int r = mbox_recv_k(id, &v);   // may block
  if(r < 0) return -1;
  if(copyout(myproc()->pagetable, uptr, (char*)&v, sizeof(v)) < 0) return -1;
  return 0;
}