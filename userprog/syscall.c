#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include <string.h>
#include "filesys/file.h"

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

  /* project 1 */

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
    case SYS_FIBONACCI:  
      check_valid_user_vaddr(f->esp + 4);
      f->eax = fibonacci(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_MAX_OF_FOUR_INT:  
      check_valid_user_vaddr(f->esp + 16);
      f->eax = max_of_four_int(*(int32_t*)(f->esp + 4), *(int32_t*)(f->esp + 8), *(int32_t*)(f->esp + 12), *(int32_t*)(f->esp + 16));
      break;

    /* project 2 */
    
    case SYS_WRITE:
      check_valid_user_vaddr(f->esp + 12);
      check_valid_user_vaddr((void*)*(uint32_t*)(f->esp + 8));
      f->eax = write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    case SYS_READ:
      check_valid_user_vaddr(f->esp + 12); 
      check_valid_user_vaddr((void*)*(uint32_t*)(f->esp + 8));
      f->eax = read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)));
      break;

    case SYS_CREATE:
      check_valid_user_vaddr(f->esp + 8); 
      f->eax = create((char*)*(uint32_t*)(f->esp + 4), *(uint32_t*)(f->esp + 8));
      break;
    case SYS_OPEN:
      check_valid_user_vaddr(f->esp + 4); 
      f->eax = open((char*)*(uint32_t*)(f->esp + 4));
      break;
    case SYS_FILESIZE:
      check_valid_user_vaddr(f->esp + 4); 
      f->eax = filesize((int)*(uint32_t*)(f->esp + 4));
      break;
      
    case SYS_CLOSE:
      check_valid_user_vaddr(f->esp + 4); 
      close((int)*(uint32_t*)(f->esp + 4));
      break;
    
    case SYS_REMOVE:
      check_valid_user_vaddr(f->esp + 4); 
      f->eax = remove((char*)*(uint32_t*)(f->esp + 4));
      break;
    case SYS_SEEK:
      check_valid_user_vaddr(f->esp + 8); 
      seek((int)*(uint32_t*)(f->esp + 4), (unsigned)*((uint32_t*)(f->esp + 8)));
      break;
    case SYS_TELL:
      check_valid_user_vaddr(f->esp + 4); 
      f->eax = tell((int)*(uint32_t*)(f->esp + 4));
      break;
  }
}

/* project 1 */

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

/* project 2*/

int read (int fd, void *buffer, unsigned length){
  lock_acquire(&file_access_lock);

  if(fd == 0){
    for(unsigned i = 0; i < length; i++){
      *(char*)(buffer + i) = input_getc();
    }

    lock_release(&file_access_lock);
    return length;
  }
  if(fd <= 2){
    lock_release(&file_access_lock);
    return -1;
  }
  if(thread_current()->fd[fd] == NULL){
    lock_release(&file_access_lock);
    return -1;
  }

  lock_release(&file_access_lock);
  return (int)file_read(thread_current()->fd[fd], buffer, length);
} 

int write (int fd, const void *buffer, unsigned length){
  lock_acquire(&file_access_lock);

  if(fd == 1){
    lock_release(&file_access_lock);
    putbuf(buffer, length);
    return length;
  } 

  if(fd <= 2){
    lock_release(&file_access_lock);
    return -1;
  }

  if(thread_current()->fd[fd] == NULL){
    lock_release(&file_access_lock);
    return -1;
  }

  int retval = (int)file_write(thread_current()->fd[fd], buffer, length);

  lock_release(&file_access_lock);
  return retval;
}

bool create (const char *file, unsigned initial_size){
  if(file == NULL){
    exit(-1);
  }

  return filesys_create(file, (off_t)initial_size);
}

int open (const char *file){
  if(file == NULL){
    exit(-1);
  }

  lock_acquire(&file_access_lock);

  struct file *fp = filesys_open(file);

  if(fp == NULL){
    lock_release(&file_access_lock);
    return -1;
  }

  int i;
  for(i = 3; i < 128; i++){
    if(thread_current()->fd[i] == NULL){
      thread_current()->fd[i] = fp;
      break;
    }
  }

  lock_release(&file_access_lock);
  if(i == 128){
    file_close(fp); 
    return -1;
  }

  return i;
}

int filesize (int fd){
  if(thread_current()->fd[fd] == NULL){
    exit(-1);
  }

  return file_length(thread_current()->fd[fd]);
}

void close (int fd){
  file_close(thread_current()->fd[fd]);
  thread_current()->fd[fd] = NULL;
}

bool remove (const char *file){
  if(file == NULL){
    exit(-1);
  }

  lock_acquire(&file_access_lock);

  bool retval = filesys_remove(file);

  lock_release(&file_access_lock);

  return retval;
}

void seek (int fd, unsigned position){
  if(thread_current()->fd[fd] == NULL){ 
    exit(-1);
  }

  file_seek(thread_current()->fd[fd], position);
}

unsigned tell (int fd){
  if(thread_current()->fd[fd] == NULL){
    exit(-1);
  }

  return (unsigned)file_tell(thread_current()->fd[fd]);
}
