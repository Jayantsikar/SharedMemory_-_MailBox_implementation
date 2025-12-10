xv6 IPC Extensions: Shared Memory and Mailbox Mechanisms

#Compilation

cd xv6
make clean
make


#Running the Shared Memory and Mailbox Test

make qemu
shmmbox_test


(This runs the combined shared-memory and mailbox test program.)

#Running the Intertwined Memory Challenge

make qemu
master


(This starts the master program, which initializes shared memory, creates mailboxes, forks two processes, and runs the full synchronization challenge.)

#Program Structure

kernel/shm.c
kernel/mbox.c
kernel/sysproc.c
kernel/syscall.c
kernel/syscall.h
kernel/defs.h
kernel/user.h
kernel/usys.pl

user/shmmbox_test.c
user/master.c
user/process.c
