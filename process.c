// user/process.c
#include "kernel/types.h"
#include "user/user.h"

#define MAXN 32
#define END  (-1)

struct Shared {
  int N, end;
  int a2b[MAXN];   // if A is at i -> B's next
  int b2a[MAXN];   // if B is at j -> A's next
  int startA, startB;
};

/* ------- single-write logging to avoid garbled lines ------- */
static int append(char *b, int o, const char *s){ while(*s) b[o++]=*s++; return o; }
static int append_int(char *b, int o, int x){
  char t[16]; int n=0;
  if(x==0){ b[o++]='0'; return o; }
  if(x<0){ b[o++]='-'; x=-x; }
  while(x){ t[n++]='0'+(x%10); x/=10; }
  while(n){ b[o++]=t[--n]; }
  return o;
}
static void line_step(char who, int cur, int next, int peer){
  char buf[96]; int n=0;
  n=append(buf,n,"Process "); buf[n++]=who; n=append(buf,n,": ");
  if(next==END){
    n=append(buf,n,"reached END\n");
  }else{
    n=append(buf,n,"at "); n=append_int(buf,n,cur);
    n=append(buf,n," -> next "); n=append_int(buf,n,next);
    n=append(buf,n," (peer "); n=append_int(buf,n,peer); n=append(buf,n,")\n");
  }
  write(1, buf, n);
}
/* ----------------------------------------------------------- */

static int atoi_safe(const char *s){
  int neg=0,x=0; if(*s=='-'){neg=1; s++;} 
  while(*s>='0' && *s<='9'){ x = x*10 + (*s-'0'); s++; }
  return neg ? -x : x;
}

int
main(int argc, char **argv)
{
  if(argc < 5){
    write(1,"usage: process <role 0|1> <shmkey> <a2bkey> <b2akey>\n",54);
    exit(1);
  }
  int role   = atoi_safe(argv[1]);   // 0 = A (send-first), 1 = B (recv-first)
  int shmkey = atoi_safe(argv[2]);
  int a2bkey = atoi_safe(argv[3]);
  int b2akey = atoi_safe(argv[4]);

  struct Shared *S = (struct Shared*)shm_get(shmkey);
  if(!S){ write(1,"process: shm_get failed\n",24); exit(1); }

  int a2b = mbox_create(a2bkey);
  int b2a = mbox_create(b2akey);
  if(a2b < 0 || b2a < 0){ write(1,"process: mbox_create failed\n",28); exit(1); }

  int mypos = (role==0) ? S->startA : S->startB;
  int done  = 0;

  for(;;){
    if(role == 0){ 
      // A: SEND -> RECV
      mbox_send(a2b, done ? END : mypos);

      int bpos;
      mbox_recv(b2a, &bpos);
      if(bpos == END){
        // symmetrical ACK so B can exit its next recv()
        mbox_send(a2b, END);
        break;
      }

      int nextA = S->b2a[bpos];
      line_step('A', mypos, nextA, bpos);
      if(nextA == END) done = 1; else mypos = nextA;

    }else{
      // B: RECV -> SEND
      int apos;
      mbox_recv(a2b, &apos);
      if(apos == END){
        // acknowledge and exit
        mbox_send(b2a, END);
        break;
      }

      int nextB = S->a2b[apos];
      line_step('B', mypos, nextB, apos);
      mbox_send(b2a, done ? END : mypos);
      

      
      if(nextB == END) done = 1; else mypos = nextB;
    }
  }

  exit(0);
}
