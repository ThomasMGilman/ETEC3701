/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#pragma once
#define SUCCESS 0
#define ENOENT 1 //no such directory entry
#define EINVAL 2 //invalid parameter
#define EMFILE 3 //too many open files
#define ENOSYS 4 //no such system call
#define DRIERR 5 //fatal error reading disk inode
#define ENOSPC 6 //No space on device
#define EFAULT 8
#define ENOTTY 9