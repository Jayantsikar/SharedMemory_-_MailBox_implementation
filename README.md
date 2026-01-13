# Shared Memory and Mailbox IPC in xv6

## Overview
This project extends the **xv6 operating system** by implementing kernel-level **Inter-Process Communication (IPC)** mechanisms. The goal is to enable efficient data sharing and reliable message passing between processes.

## Implemented Features

### 1. Shared Memory
- Added new system calls to create, attach, and detach shared memory regions.
- Implemented kernel data structures to track shared pages and reference counts.
- Ensured safe cleanup of shared memory on process `exit()` and `exec()`.
- Verified correctness using multi-process test programs.

### 2. Mailboxes
- Implemented bounded mailboxes with FIFO message ordering.
- Supported blocking `send` and `receive` operations with proper synchronization.
- Enabled reliable process-to-process communication using mailbox keys.

### 3. Intertwined Memory Challenge
- Built a concurrent application using shared memory and mailboxes.
- Two cooperating processes navigate an intertwined path stored in shared memory.
- Designed an asymmetric communication protocol to avoid deadlocks.
- Implemented clean termination using an `END` message handshake.

## Key Learnings
- Kernel-level IPC design and synchronization
- Shared memory lifecycle management
- Deadlock avoidance using communication protocols
- Practical use of IPC primitives in concurrent programs

## References
- xv6: A Simple, Unix-like Teaching Operating System
- *Operating System Concepts* by Silberschatz, Galvin, and Gagne
