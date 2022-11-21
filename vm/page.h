#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"

/* type of vm_enrtry */
#define VM_BIN  0   /* from binary file */
#define VM_SWAP 1   /* from swap space */

struct vm_entry{
    int type;               /* type of vm_entry */
    void *vaddr;            /* virtual address which vm manages */
    bool writable;          /* true if vaddr writable */
    bool is_loaded;         /* true if loaded on memory */
    struct file* file;      /* file maped with vadddr */
    size_t offset;          /* file offset */
    size_t read_bytes;      /* data size written on virtual page */
    size_t zero_bytes;      /**/
    size_t swap_slot;       /* for swapping */
    struct hash_elem elem;  /* hash table element */
};

struct page{
    void *kaddr;
    struct vm_entry *vme;
};

void vm_init(struct hash *);
bool vm_insert_entry(struct hash *, struct vm_entry *);
bool vm_delete_entry(struct hash *, struct vm_entry *);
struct vm_entry *vm_find_entry(void *);
void vm_destroy(struct hash *);
bool load_file(void *kaddr, struct vm_entry *vme);

#endif