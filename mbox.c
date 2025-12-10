#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

struct mbox {
  int used;
  int key;
  int id;                 // index in table
  struct spinlock lock;
  int buf[MBOX_CAP];
  int head, tail, cnt;    // circular queue
};

static struct {
  struct spinlock lock;
  struct mbox box[NMB];
  int inited;
} mbs;

static void
mb_init(void)
{
  if(!mbs.inited){
    initlock(&mbs.lock, "mb_tbl");
    for(int i=0;i<NMB;i++){
      mbs.box[i].used = 0;
      initlock(&mbs.box[i].lock, "mb");
    }
    mbs.inited=1;
  }
}

static int
find_mbox_by_key(int key)
{
  for(int i=0;i<NMB;i++)
    if(mbs.box[i].used && mbs.box[i].key==key) return i;
  return -1;
}

static int
find_mbox_free(void)
{
  for(int i=0;i<NMB;i++)
    if(!mbs.box[i].used) return i;
  return -1;
}

int
mbox_create_k(int key)
{
  mb_init();
  acquire(&mbs.lock);
  int idx = find_mbox_by_key(key);
  if(idx >= 0){ release(&mbs.lock); return idx; }
  idx = find_mbox_free();
  if(idx < 0){ release(&mbs.lock); return -1; }

  struct mbox *m = &mbs.box[idx];
  m->used = 1;
  m->key = key;
  m->id = idx;
  m->head = m->tail = m->cnt = 0;
  release(&mbs.lock);
  return idx;
}

int
mbox_send_k(int id, int msg)
{
  if(id < 0 || id >= NMB) return -1;
  struct mbox *m = &mbs.box[id];
  if(!m->used) return -1;

  acquire(&m->lock);
  while(m->cnt == MBOX_CAP)
    sleep(m, &m->lock);          // wait for space
  m->buf[m->tail] = msg;
  m->tail = (m->tail + 1) % MBOX_CAP;
  m->cnt++;
  wakeup(m);                     // wake one+ receivers
  release(&m->lock);
  return 0;
}

int
mbox_recv_k(int id, int *out)
{
  if(id < 0 || id >= NMB) return -1;
  struct mbox *m = &mbs.box[id];
  if(!m->used) return -1;

  acquire(&m->lock);
  while(m->cnt == 0)
    sleep(m, &m->lock);          // wait for data
  int v = m->buf[m->head];
  m->head = (m->head + 1) % MBOX_CAP;
  m->cnt--;
  wakeup(m);                     // wake one+ senders
  release(&m->lock);

  *out = v;
  return 0;
}
