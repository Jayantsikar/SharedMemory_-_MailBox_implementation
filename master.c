// user/master.c
#include "kernel/types.h"
#include "user/user.h"

// Keys (can be any ints; must match in both processes)
#define SHMKEY   2000
#define A2BKEY   2001
#define B2AKEY   2002

// Shared page layout
#define MAXN 32
#define END  (-1)

struct Shared {
  int N;
  int end;       // END marker
  int a2b[MAXN]; // If A at idx i, B's next is a2b[i]
  int b2a[MAXN]; // If B at idx j, A's next is b2a[j]
  int startA;    // A's starting index
  int startB;    // B's starting index
};

static void itoa(int x, char *buf){
  char t[16]; int n=0, k=0;
  if(x==0){ buf[0]='0'; buf[1]=0; return; }
  if(x<0){ buf[k++]='-'; x=-x; }
  while(x){ t[n++] = '0' + (x%10); x/=10; }
  while(n) buf[k++] = t[--n];
  buf[k]=0;
}

int
main(int argc, char **argv)
{
  // 1) Create SHM once and map it.
  int sid = shm_create(SHMKEY);
  if(sid < 0){ printf("master: shm_create failed\n"); exit(1); }
  struct Shared *S = (struct Shared*)shm_get(SHMKEY);
  if(!S){ printf("master: shm_get failed\n"); exit(1); }

  // 2) Initialize intertwined paths in shared memory.
  S->N = 16;                // just informational
  S->end = END;
  for(int i=0;i<MAXN;i++){ S->a2b[i] = END; S->b2a[i] = END; }

  // Longer intertwined route:
  // A positions (time order): 0,3,7,12,5,9,14,2,8,11,4,6, then END
  // B positions (time order): 0,4,1, 6,10,13,15,3,7, 2,5,12, then END
  // Fill the mapping rules:
  int Aseq[] = {0,3,7,12,5,9,14,2,8,11,4,6};
  int Bseq[] = {0,4,1,6,10,13,15,3,7,2,5,12};
  int L = sizeof(Aseq)/sizeof(Aseq[0]); // == sizeof(Bseq)/sizeof(Bseq[0])

  // When A is at Aseq[k], B’s next must be Bseq[k+1]
  for(int k=0;k<L-1;k++) S->a2b[Aseq[k]] = Bseq[k+1];
  S->a2b[Aseq[L-1]] = END;

  // When B is at Bseq[k], A’s next must be Aseq[k+1]
  for(int k=0;k<L-1;k++) S->b2a[Bseq[k]] = Aseq[k+1];
  S->b2a[Bseq[L-1]] = END;

  // Starts
  S->startA = Aseq[0];   // 0
  S->startB = Bseq[0];   // 0

  // 3) Create the two mailboxes (keys -> ids are resolved in process)
  if(mbox_create(A2BKEY) < 0 || mbox_create(B2AKEY) < 0){
    printf("master: mbox_create failed\n"); exit(1);
  }

  // 4) Fork & exec the two processes with role and keys
  for(int role=0; role<2; role++){
    int pid = fork();
    if(pid == 0){
      char rbuf[8], sk[12], k1[12], k2[12];
      itoa(role, rbuf); itoa(SHMKEY, sk); itoa(A2BKEY, k1); itoa(B2AKEY, k2);
      char *pargv[] = { "process", rbuf, sk, k1, k2, 0 };
      if(exec("process", pargv) < 0){
        printf("master: exec process failed\n"); exit(1);
      }
    } else if(pid < 0){
      printf("master: fork failed\n"); exit(1);
    }
  }

  // 5) Wait for both to finish
  wait(0); wait(0);
  shm_close(SHMKEY);
  printf("master: both processes finished\n");
  exit(0);
}
