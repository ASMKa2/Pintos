#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"
#include "filesys/file.h"
#include "lib/kernel/list.h"
#include "threads/palloc.h"
#include "threads/synch.h"

/* type of vm_enrtry */
#define VM_BIN  0   /* from binary file */
#define VM_SWAP 1   /* from swap space */
#define VM_FILE 2   /* from file */

struct vm_entry{
    int type;                   /* type of vm_entry */
    void *vaddr;                /* virtual address which vm manages */
    bool writable;              /* true if vaddr writable */
    bool is_loaded;             /* true if loaded on memory */
    struct file* file;          /* file maped with vadddr */
    size_t offset;              /* file offset */
    size_t read_bytes;          /* data size written on virtual page */
    size_t zero_bytes;          /* data size of 0's written */
    size_t swap_slot;           /* for swapping */
    struct hash_elem elem;      /* hash table element */

    struct list_elem mmap_elem; /* mmap list element */
};

struct page{
    void *kaddr;                    /* physical address of page */
    struct vm_entry *vme;           /* page mapped */
    struct thread *thread;          /* thread using page */
    struct list_elem plist_elem;    /* page list element */
};

struct list plist;                  /* page list */
struct lock plist_lock;             /* lock for page list */
struct page *victim;                /* pointer to find victim page */

struct mmap_file{
    int mapid;                      /* map id returned when mmap() success */
    struct file *file;              /* mapped file */
    struct list_elem elem;          /* mmap_files list */
    struct list vm_entry_list;      /* mmap_file's vm_entry list*/
};

void vm_init(struct hash *);                            /* initialize vm */
bool vm_insert_entry(struct hash *, struct vm_entry *); /* insert vm_entry to vm */
bool vm_delete_entry(struct hash *, struct vm_entry *); /* delete vm_entry from vm */
struct vm_entry *vm_find_entry(void *);                 /* find vm_entry from vm by virtual address */
void vm_destroy(struct hash *);                         /* destroy vm */
bool load_file(void *kaddr, struct vm_entry *vme);      /* load file starting from kaddr, add to vme */

#endif