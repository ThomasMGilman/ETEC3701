#pragma once

#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_CLOSE 3
#define SYSCALL_EXIT 4

void syscall_handler(unsigned* ptr);

int syscall(int p0, int p1, int p2, int p3);