#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  // SHM test
  int id = shm_create(42);
  if(id < 0){ printf("shm_create fail\n"); exit(1); }
  uchar *p = (uchar*)shm_get(42);
  if(!p){ printf("shm_get fail\n"); exit(1); }
  p[0] = 77;

  int pid = fork();
  if(pid == 0){
    uchar *q = (uchar*)shm_get(42);
    printf("child sees %d\n", q[0]);   // expect 77
    q[0] = 99;
    shm_close(42);
    exit(0);
  }
  wait(0);
  printf("parent now sees %d\n", p[0]); // expect 99
  shm_close(42);

  // MBOX test
  int mid = mbox_create(7);
  if(mid < 0){ printf("mbox_create fail\n"); exit(1); }
  int c = fork();
  if(c == 0){
    int v;
    for(int i=0;i<5;i++){
      mbox_recv(mid, &v);
      printf("child got %d\n", v);
    }
    exit(0);
  } else {
    for(int i=0;i<5;i++){
      mbox_send(mid, 100+i);
    }
    wait(0);
  }
  exit(0);
}



// // user/shmmbox_stress.c
// #include "kernel/types.h"
// #include "user/user.h"

// // ---- config (tweak freely) ----
// #define K_SHM        90     // shared memory key
// #define K_LOCK       910    // mailbox used as a binary lock
// #define K_MQ         911    // mailbox for FIFO burst test
// #define CHILDREN       6
// #define INCS        1000
// #define FIFO_N       100

// static void die(const char *m){ printf("%s\n", m); exit(1); }
// static void lock_acq(int mid){ int tok; if(mbox_recv(mid,&tok)<0) die("lock recv"); }
// static void lock_rel(int mid){ if(mbox_send(mid,1)<0) die("lock send"); }

// int
// main(void)
// {
//   // ---------- SHM counter with mailbox lock ----------
//   if(shm_create(K_SHM) < 0) die("shm_create");
//   int *cnt = (int*)shm_get(K_SHM);
//   if(!cnt) die("shm_get");
//   *cnt = 0;

//   int lock = mbox_create(K_LOCK);
//   if(lock < 0) die("mbox_create lock");
//   // seed binary lock
//   if(mbox_send(lock, 1) < 0) die("lock seed");

//   for(int i=0;i<CHILDREN;i++){
//     int pid = fork();
//     if(pid < 0) die("fork");
//     if(pid == 0){
//       // child: attach to shm and increment with lock
//       volatile int *c = (int*)shm_get(K_SHM);
//       if(!c) die("child shm_get");
//       for(int k=0;k<INCS;k++){
//         lock_acq(lock);
//         int v = *c;
//         *c = v + 1;
//         lock_rel(lock);
//       }
//       exit(0);
//     }
//   }
//   for(int i=0;i<CHILDREN;i++) wait(0);

//   int expect = CHILDREN * INCS;
//   if(*cnt != expect){
//     printf("SHM counter FAIL: got %d expect %d\n", *cnt, expect);
//     exit(1);
//   }
//   printf("SHM counter PASS: %d\n", *cnt);

//   // ---------- Mailbox FIFO burst (ordering) ----------
//   int mq = mbox_create(K_MQ);
//   if(mq < 0) die("mbox_create mq");

//   int c = fork();
//   if(c < 0) die("fork fifo");
//   if(c == 0){
//     // receiver
//     int ok = 1, v;
//     for(int i=0;i<FIFO_N;i++){
//       if(mbox_recv(mq, &v) < 0) die("recv");
//       if(v != i) ok = 0;
//     }
//     if(!ok) die("FIFO order broken");
//     printf("Mailbox FIFO PASS (%d msgs)\n", FIFO_N);
//     exit(0);
//   } else {
//     // sender (burst)
//     for(int i=0;i<FIFO_N;i++){
//       if(mbox_send(mq, i) < 0) die("send");
//     }
//     wait(0);
//   }

//   // cleanup (optional; kernel should also auto-detach on exit if you added that)
//   if(shm_close(K_SHM) < 0) die("shm_close");

//   printf("shmmbox_stress: ALL PASS\n");
//   exit(0);
// }
