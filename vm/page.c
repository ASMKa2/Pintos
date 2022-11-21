#include <debug.h>
#include <string.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "filesys/file.h"

static unsigned vm_hash_func(const struct hash_elem *, void *);
static bool vm_cmp_vaddr(const struct hash_elem *, const struct hash_elem *);
static void vm_destroy_func(struct hash_elem *, void *);

void vm_init(struct hash *vm){
    hash_init(vm, vm_hash_func, vm_cmp_vaddr, NULL);
}

bool vm_insert_entry(struct hash *vm, struct vm_entry *vme){
    if(hash_insert(vm, &vme->elem) == NULL){
        return true;
    }
    return false;
}

bool vm_delete_entry(struct hash *vm, struct vm_entry *vme){
    if(hash_delete(vm, &vme->elem) == NULL){
        return true;
    }
    return false;
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux UNUSED){
    return hash_int((int)hash_int(hash_entry(e, struct vm_entry, elem)->vaddr));
}

static bool vm_cmp_vaddr(const struct hash_elem *e1, const struct hash_elem *e2){
    int vaddr1 = (int)hash_int(hash_entry(e1, struct vm_entry, elem)->vaddr);
    int vaddr2 = (int)hash_int(hash_entry(e2, struct vm_entry, elem)->vaddr);

    return vaddr1 < vaddr2;
}

struct vm_entry *vm_find_entry(void *vaddr){
    struct vm_entry vme;
    vme.vaddr = pg_round_down(vaddr);
    
    struct hash_elem *h = hash_find(&thread_current()->vm, &vme.elem);

    if(h != NULL){
        return hash_entry(h, struct vm_entry, elem);
    }
    return NULL;
}

void vm_destroy(struct hash *vm){
    hash_destroy(vm, vm_destroy_func);
}

static void vm_destroy_func(struct hash_elem *e, void *aux UNUSED){
    struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
    
    if(vme->is_loaded){
        void *paddr = pagedir_get_page(thread_current()->pagedir, vme->vaddr);
        palloc_free_page(paddr);
        pagedir_clear_page(thread_current()->pagedir, vme->vaddr);
    }

    free(vme);
}

bool load_file(void *kaddr, struct vm_entry *vme){
    int read = file_read_at(vme->file, kaddr, vme->read_bytes, vme->offset);
    if(read == (int)vme->read_bytes){
        return false;
    }

    memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);
    return true;
}