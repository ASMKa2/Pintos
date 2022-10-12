#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

void check_valid_user_vaddr(const void *addr){
  if(!is_user_vaddr(addr)){
    exit(-1);
  }
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  check_valid_user_vaddr(f->esp);

  switch(*(uint32_t*)(f->esp)){
    case SYS_HALT: 
      halt();
      break;
    case SYS_EXIT:
      check_valid_user_vaddr(f->esp + 4); 
      exit(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_EXEC:
      check_valid_user_vaddr(f->esp + 4);
      f->eax = exec((const char*)*(uint32_t*)(f->esp + 4));
      break; 
    case SYS_WAIT: 
      check_valid_user_vaddr(f->esp + 4);
      f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WRITE:
      check_valid_user_vaddr(f->esp + 12);
      f->eax = write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    case SYS_READ:
      check_valid_user_vaddr(f->esp + 12); 
      f->eax = read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    case SYS_FIBONACCI:  
      check_valid_user_vaddr(f->esp + 4);
      f->eax = fibonacci(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_MAX_OF_FOUR_INT:  
      check_valid_user_vaddr(f->esp + 16);
      f->eax = max_of_four_int(*(int32_t*)(f->esp + 4), *(int32_t*)(f->esp + 8), *(int32_t*)(f->esp + 12), *(int32_t*)(f->esp + 16));
      break;
  }
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){
  struct thread *cur_t = thread_current(); 
  printf("%s: exit(%d)\n", cur_t->name, status);

  cur_t->exit_status = status; 

  thread_exit(); 
}

pid_t exec (const char *file){
  char parsed[256];

  int i;
  for(i = 0; file[i] != '\0' && file[i] != ' '; i++){
    parsed[i] = file[i];
  }
  parsed[i] = '\0';

  struct file *fp = filesys_open (parsed);
  if (fp == NULL) {
    return -1;
  }

  tid_t tid = process_execute(file);

  if(tid == TID_ERROR){
    return -1;
  }

  return (pid_t)tid; 
}

int wait (pid_t pid){
  return process_wait((tid_t)pid); 
}

int read (int fd, void *buffer, unsigned length){
  switch(fd){
    case 0:
      for(int i = 0; i < length; i++){
        *(char*)(buffer + i) = input_getc();
      }
      return length;
    default:
      return -1;
  }
} 

int write (int fd, const void *buffer, unsigned length){
  if(fd == 1){
    putbuf(buffer, length);
    return length;
  }

  return -1;
}

int fibonacci (int num){
  if(num < 2){
    return num;
  }

  int n1 = 0;
  int n2 = 1;
  int tmp;

  for(int i = 2; i <= num; i++){
    tmp = n2;
    n2 += n1;
    n1 = tmp;
  }

  return n2;
}

int max_of_four_int (int num1, int num2, int num3, int num4){
  int res = num1 > num2 ? num1 : num2;
  res = num3 > res ? num3 : res;
  
  return num4 > res ? num4 : res;
}