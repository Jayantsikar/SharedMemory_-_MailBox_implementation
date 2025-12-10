struct stat;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int getyear(void);
int top(void);
int set_priority(int n);
int get_priority(void);
int get_rtime(void);
int setburst(int);

int    shm_create(int key);
void*  shm_get(int key);
int    shm_close(int key);

int    mbox_create(int key);
int    mbox_send(int id, int msg);
int    mbox_recv(int id, int *msg);



// The CPU jumps to the address in stvec (set to TRAMPOLINE+uservec), landing in trampoline.S:uservec.

// That assembly saves all user registers into the process’s trapframe, switches to the kernel page table, and jumps into kernel/usertrap().

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...) __attribute__ ((format (printf, 2, 3)));
void printf(const char*, ...) __attribute__ ((format (printf, 1, 2)));
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);

// umalloc.c
void* malloc(uint);
void free(void*);
