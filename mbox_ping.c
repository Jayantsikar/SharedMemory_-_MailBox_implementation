// user/mbox_ping.c
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int a2b = mbox_create(100);
  int b2a = mbox_create(101);

  int pid = fork();
  if(pid == 0){
    // B
    for(int i=0;i<3;i++){
      int v; mbox_recv(a2b, &v);
      mbox_send(b2a, v+1);
    }
    exit(0);
  } else if(pid > 0){
    // A
    for(int i=0;i<3;i++){
      mbox_send(a2b, i);
      int v; mbox_recv(b2a, &v);
      printf("ping %d -> pong %d\n", i, v);
    }
    wait(0);
    exit(0);
  }
  exit(1);
}


// // user/mbox_ring.c
// #include "kernel/types.h"
// #include "user/user.h"

// // Number of processes in the ring and laps the token should make
// #define K     6      // ring size (>=2)
// #define LAPS  20     // how many full laps the token makes

// // Base key for the K mailboxes (each hop has its own mailbox)
// #define BASEKEY  500

// int
// main(void)
// {
//   int in_mbox[K], out_mbox[K];

//   // Create mailboxes for each hop i -> (i+1)%K
//   for (int i = 0; i < K; i++) {
//     int in  = mbox_create(BASEKEY + i);           // inbox for node i
//     int out = mbox_create(BASEKEY + ((i+1)%K));   // outbox to next node
//     if (in < 0 || out < 0) { printf("mbox_create fail\n"); exit(1); }
//     in_mbox[i]  = in;
//     out_mbox[i] = out;
//   }

//   // Fork K children; each is one node in the ring
//   for (int i = 0; i < K; i++) {
//     int pid = fork();
//     if (pid == 0) {
//       // Child i: repeatedly receive, transform, forward
//       int in  = in_mbox[i];
//       int out = out_mbox[i];

//       for (;;) {
//         int v;
//         mbox_recv(in, &v);             // blocks until a value arrives

//         if (v == -1) {                 // termination token; forward and exit
//           mbox_send(out, -1);
//           exit(0);
//         }

//         v++;                           // advance the token (count total hops)

//         if (v == K * LAPS) {
//           // completed all laps; inject termination token
//           mbox_send(out, -1);
//           exit(0);
//         } else {
//           mbox_send(out, v);           // pass to next node
//         }
//       }
//     } else if (pid < 0) {
//       printf("fork failed\n");
//       exit(1);
//     }
//   }

//   // Seed the ring: put token=0 into node 0's inbox
//   mbox_send(in_mbox[0], 0);

//   // Reap all children
//   for (int i = 0; i < K; i++) wait(0);

//   printf("Mailbox ring PASS (K=%d, LAPS=%d)\n", K, LAPS);
//   exit(0);
// }
