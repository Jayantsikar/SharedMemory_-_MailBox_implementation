# Shared Memory and Mailbox IPC in xv6

## Overview
This project extends the **xv6 operating system** by implementing kernel-level **Inter-Process Communication (IPC)** mechanisms. It adds support for **shared memory** and **mailboxes**, enabling efficient data sharing and reliable message passing between processes. These primitives are further used to build concurrent applications demonstrating synchronization and deadlock-free communication.

---

## Features Implemented

### 1. Shared Memory
- Implemented shared memory support inside the xv6 kernel.
- Added system calls to create, attach, and detach shared memory regions.
- Maintained kernel data structures to track shared pages and reference counts.
- Ensured safe cleanup during `exit()` and `exec()`.

### 2. Mailboxes
- Implemented bounded mailboxes with FIFO semantics.
- Supported blocking `send` and `receive` operations.
- Enabled reliable inter-process communication using mailbox keys.

### 3. User-Level Applications
- `shmmbox_test.c` tests shared memory visibility and mailbox communication.
- `mbox_ping.c` demonstrates mailbox-based ping-pong messaging.
- `master.c` and `process.c` implement the Intertwined Memory Challenge using shared memory and mailboxes with deadlock avoidance.

---

## Repository Structure

```
.
├── Makefile
├── README.md
├── report.pdf
├── defs.h
├── param.h
├── syscall.h
├── syscall.c
├── sysproc.c
├── proc.c
├── exec.c
├── shm.c
├── mbox.c
├── user.h
├── usys.pl
├── shmmbox_test.c
├── mbox_ping.c
├── master.c
├── process.c
```

---

## How to Run

### 1. Setup xv6
```bash
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
```

### 2. Copy Project Files
- Copy kernel-related files into `kernel/`:
  - `shm.c`, `mbox.c`
  - `defs.h`, `param.h`
  - `syscall.c`, `syscall.h`, `sysproc.c`
  - `proc.c`, `exec.c`

- Copy user-related files into `user/`:
  - `shmmbox_test.c`
  - `mbox_ping.c`
  - `master.c`
  - `process.c`
  - `user.h`
  - `usys.pl`

Replace the corresponding xv6 files where required.

### 3. Update Makefile
Add the following entries to `UPROGS` in the xv6 `Makefile`:
```makefile
UPROGS += \
	$U/_shmmbox_test \
	$U/_mbox_ping \
	$U/_master
```

### 4. Build and Run xv6
```bash
make clean
make qemu
```

---

## Running the Programs (inside xv6 shell)

```bash
shmmbox_test
```

```bash
mbox_ping
```

```bash
master
```

---

## Expected Output
- Shared memory updates are visible across multiple processes.
- Mailboxes deliver messages in FIFO order with correct blocking behavior.
- Intertwined memory program completes without deadlock and both processes reach `END`.

---

## Key Learnings
- Kernel-level IPC design in xv6
- Shared memory lifecycle management
- Blocking message passing using mailboxes
- Deadlock avoidance via asymmetric protocols

---

## References
- xv6: A Simple, Unix-like Teaching Operating System
- Operating System Concepts — Silberschatz, Galvin, and Gagne
