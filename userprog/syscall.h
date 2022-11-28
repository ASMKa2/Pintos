#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"
#include "threads/vaddr.h"

void check_valid_user_vaddr(const void *addr);

void syscall_init (void);

void halt (void) NO_RETURN;
void exit (int status) NO_RETURN;
pid_t exec (const char *file);
int wait (pid_t);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
int fibonacci (int num);
int max_of_four_int (int num1, int num2, int num3, int num4);
mapid_t mmap(int fd, void *addr);
void munmap(mapid_t mapping);

#endif /* userprog/syscall.h */
