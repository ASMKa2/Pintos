#include <debug.h>
#include <string.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "vm/frame.h"

static unsigned vm_hash_func(const struct hash_elem *, void *aux UNUSED);
static bool vm_cmp_vaddr(const struct hash_elem *, const struct hash_elem *, void *aux UNUSED);
static void vm_destroy_func(struct hash_elem *, void *);

/* initialize vm */
void vm_init(struct hash *vm){
    hash_init(vm, vm_hash_func, vm_cmp_vaddr, NULL);
}

/* insert vm_entry to vm */
bool vm_insert_entry(struct hash *vm, struct vm_entry *vme){
    if(hash_insert(vm, &vme->elem) == NULL){
        return true;
    }
    return false;
}

/* delete vm_entry from vm */
bool vm_delete_entry(struct hash *vm, struct vm_entry *vme){
    if(hash_delete(vm, &vme->elem) != NULL){
        free(vme);
        return true;
    }
    free(vme);
    return false;
}

/* hash function for vm */
static unsigned vm_hash_func(const struct hash_elem *e, void *aux UNUSED){
	struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
	return hash_int((int)vme->vaddr);
}

/* compare to hash elements by their vaddr */
static bool vm_cmp_vaddr(const struct hash_elem *e1, const struct hash_elem *e2, void *aux UNUSED){
    return hash_entry(e1, struct vm_entry, elem)->vaddr < hash_entry(e2, struct vm_entry, elem)->vaddr;
}

/* find vm_entry from vm by virtual address */
struct vm_entry *vm_find_entry(void *vaddr){
    struct vm_entry vme;
    vme.vaddr = pg_round_down(vaddr);
    
    struct hash_elem *h = hash_find(&thread_current()->vm, &vme.elem);

    if(h != NULL){
        return hash_entry(h, struct vm_entry, elem);
    }
    return NULL;
}

/* destory vm */
void vm_destroy(struct hash *vm){
    hash_destroy(vm, vm_destroy_func);
}

/* actual vm destroy */
static void vm_destroy_func(struct hash_elem *e, void *aux UNUSED){
    struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
    
    if(vme->is_loaded){
        void *paddr = pagedir_get_page(thread_current()->pagedir, vme->vaddr);
        free_page(paddr);
        pagedir_clear_page(thread_current()->pagedir, vme->vaddr);
    }

    free(vme);
}

/* load file starting from kaddr, add to vm_entry*/
bool load_file(void *kaddr, struct vm_entry *vme){
    int read = file_read_at(vme->file, kaddr, vme->read_bytes, vme->offset);
    if(read != (int)vme->read_bytes){
        return false;
    }

    memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);

    return true;
}