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
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "vm/page.h"
#include "vm/frame.h"

static void syscall_handler (struct intr_frame *);
void check_valid_buffer(void *buffer, unsigned size, bool to_write);
static void do_munmap(struct mmap_file *mfile);

void check_valid_user_vaddr(const void *addr){
  if(!is_user_vaddr(addr)){
    exit(-1);
  }
}

void check_valid_buffer(void *buffer, unsigned size, bool to_write){
  struct vm_entry *vme;
  for(int i = 0; i < size; i++){
    check_valid_user_vaddr(buffer + i);
    vme = vm_find_entry(buffer + i);
    if(vme != NULL){
      if(!vme->writable && to_write){
        exit(-1);
      }
    }
    else{
      exit(-1);
    }
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
      check_valid_buffer((void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)), false);
      f->eax = write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    case SYS_READ:
      check_valid_user_vaddr(f->esp + 12); 
      check_valid_user_vaddr((void*)*(uint32_t*)(f->esp + 8));
      check_valid_buffer((void*)*(uint32_t*)(f->esp + 8), (unsigned)*((uint32_t*)(f->esp + 12)), true);
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
    case SYS_MMAP:
      check_valid_user_vaddr(f->esp + 8);
      check_valid_user_vaddr((void*)*(uint32_t*)(f->esp + 8));
      f->eax = mmap((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8));
      break;
    case SYS_MUNMAP:
      check_valid_user_vaddr(f->esp + 4);
      check_valid_user_vaddr((void*)*(uint32_t*)(f->esp + 4));
      munmap((int)*(uint32_t*)(f->esp + 4));
      break;
    default:
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

  int retval = (int)file_read(thread_current()->fd[fd], buffer, length);

  lock_release(&file_access_lock);

  return retval;
} 

int write (int fd, const void *buffer, unsigned length){
  lock_acquire(&file_access_lock);

  if(fd == 1){
    putbuf(buffer, length);
    
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

  int retval = (int)file_write(thread_current()->fd[fd], buffer, length);

  lock_release(&file_access_lock);
  return retval;
}

bool create (const char *file, unsigned initial_size){
  if(file == NULL){
    exit(-1);
  }

  lock_acquire(&file_access_lock);

  bool success = filesys_create(file, (off_t)initial_size);

  lock_release(&file_access_lock);

  return success;
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

  if(i == 128){
    file_close(fp); 
    lock_release(&file_access_lock);

    return -1;
  }

  lock_release(&file_access_lock);

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
  if(thread_current()->fd[fd] == NULL || position < 0){ 
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


mapid_t mmap(int fd, void *addr){
  /* map file fd to current process's vm
  start from address addr of file fd
  return mapping id if successed, -1 if failed
  */

  /*
  0. check if parameter is valid 
  1. file_reopen() 
  2. allocate mapid
  3. mmap_file creation & initialization
  4. vm_entry creation & initialization
  5. return mapid
  */ 

  /* check if parameter is valid */
  if(fd <= 2){
    exit(-1);
  }
 
  struct file *fp = thread_current()->fd[fd];

  /* check if file descriptor is valid */
  if(fp == NULL){
    exit(-1);
  }

  /* check if address is valid */
  if(pg_round_down(addr) != addr || addr == NULL){
    return -1;
  }

  /* file_reopen() to guarantee munmap() */
  fp = file_reopen(fp);

  if(fp == NULL){
    return -1;
  }

  /*create and initialize mmap_file */
  struct mmap_file *mfile = (struct mmap_file *)malloc(sizeof(struct mmap_file));
  
  if(mfile == NULL){
    return -1;
  }

  /* initialize mfile */
  mfile->mapid = (thread_current()->mapid)++;
  mfile->file = fp;
  
  list_init(&(mfile->vm_entry_list));

  /* create and initialize vm_entry */
  off_t ofs = 0;
  uint32_t read_bytes = file_length(fp);
  uint32_t zero_bytes = (PGSIZE - read_bytes % PGSIZE) % PGSIZE;
  
  file_seek (fp, ofs);

  while(read_bytes > 0 || zero_bytes > 0){
    /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
    size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
    size_t page_zero_bytes = PGSIZE - page_read_bytes;

    /* allocate vm_entry */
    struct vm_entry *vme = malloc(sizeof(struct vm_entry));
    if(vme == NULL){ /* failed to allocate memory */
      return -1;
    }

    /* initialize vm_entry */
    vme->type = VM_FILE;
    vme->writable = true;
    vme->is_loaded = false;
    vme->file = fp;
    vme->offset = ofs;
    vme->read_bytes = page_read_bytes;
    vme->zero_bytes = page_zero_bytes;
    vme->vaddr = addr;

    if(!vm_insert_entry(&thread_current()->vm, vme)){
      return -1;
    }
    /* insert vm_entry to mfile's vm_entry_list */
    list_push_back(&mfile->vm_entry_list, &vme->mmap_elem);
    
    /* Advance. */
    read_bytes -= page_read_bytes;
    zero_bytes -= page_zero_bytes;
    ofs += page_read_bytes;
    addr += PGSIZE;
  }

  list_push_back(&thread_current()->mmap_list, &mfile->elem);
  return mfile->mapid;
}

void munmap(mapid_t mapping){
  /* 
  1. iterate mmap_list until find mapid
  2. remove vm_entry
  3. remove page table entry
  4. remove mmap_file
  5. file_close()

  2, 3 in do_munmap()
  */

  /* iterate mmap_list to find mapid */
  struct list *mmap_list = &thread_current()->mmap_list;
  
  struct list_elem *e;
  struct mmap_file *mfile;
  struct file *fp;
  struct list_elem *tmp;

  for(e = list_begin(mmap_list); e != list_end(mmap_list); e = list_next(e)){      
    mfile = list_entry(e, struct mmap_file, elem);

    if(mapping == -1 || mfile->mapid == mapping){

      fp = mfile->file;

      /* remove vm_entry's in mmap_file */
      do_munmap(mfile);

      /* close file */
      file_close(fp);

      tmp = list_prev(e);
      list_remove(e);
      e = tmp;
      
      /* free mmap_file */
      free(mfile);
      /* mapping == -1 means the process will exit, so have to remove all */
      if(mapping != -1){
        break;
      }
    }
  }

}

static void do_munmap(struct mmap_file *mfile){
  /* 
  2. remove vm_entry
  3. remove page table entry
  */

  struct list *vm_entry_list = &mfile->vm_entry_list;

  struct list_elem *e;
  struct vm_entry *vme;
  void *paddr;

  for(e = list_begin(vm_entry_list); e != list_end(vm_entry_list);){
    vme = list_entry(e, struct vm_entry, mmap_elem);

    if(vme->is_loaded){

      paddr = pagedir_get_page(thread_current()->pagedir, vme->vaddr);
      
      /* if vme is dirty, write */
      if(pagedir_is_dirty(thread_current()->pagedir, vme->vaddr)){
        lock_acquire(&file_access_lock);
        file_write_at(vme->file, vme->vaddr, vme->read_bytes, vme->offset);
        lock_release(&file_access_lock);
      }

      /* if loaded, remove PTE */
      pagedir_clear_page(thread_current()->pagedir, vme->vaddr);

      free_page(paddr);
    }

    e = list_remove(e);

    /* remove vm entry from vm (hash table) */
    vm_delete_entry(&thread_current()->vm, vme);
  }
}