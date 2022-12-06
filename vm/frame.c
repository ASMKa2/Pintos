#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "lib/kernel/list.h"
#include "userprog/pagedir.h"

static struct list_elem *get_next_victim(void);

/* init page list */
void plist_init(void){
    list_init(&plist);
    lock_init(&plist_lock);
    victim = NULL;
}

/* add page to page list */
void add_page_to_plist(struct page *page){
    if(page == NULL){
        return;
    }

    lock_acquire(&plist_lock);
    list_push_back(&plist, &page->plist_elem);
    lock_release(&plist_lock);
}

/* delete page from page list */
void del_page_from_plist(struct page *page){
    if(page == NULL){
        return;
    }

    if(victim == page){
        victim = list_entry(list_remove(&page->plist_elem), struct page, plist_elem);
    }
    else{
        list_remove(&page->plist_elem);
    }
}

/* allocate a page by given flags*/
struct page *alloc_page(enum palloc_flags flags){
    if((flags & PAL_USER) == 0){
        return NULL;
    }

    struct page *page = (struct page *)malloc(sizeof(struct page));
    if(page == NULL){
        return NULL;
    }
    
    page->kaddr = palloc_get_page(flags);

    while(page->kaddr == NULL){
        try_to_free_pages();
        page->kaddr = palloc_get_page(flags);
    }

    page->thread = thread_current();

    add_page_to_plist(page);
    
    return page;
}

/* find the page of the given physical address 
    and free it. invokes __free_page */
void free_page(void *kaddr){
    struct list_elem *e;

    lock_acquire(&plist_lock);

    for(e = list_begin(&plist); e != list_end(&plist); e = list_next(e)){
        if(list_entry(e, struct page, plist_elem)->kaddr == kaddr){
            __free_page(list_entry(e, struct page, plist_elem));
            break;
        }
    }

    lock_release(&plist_lock);
}

/* free page */
void __free_page(struct page *page){
    palloc_free_page(page->kaddr);

    del_page_from_plist(page);

    free(page);
}

/*  move victim to next page in the page list it is able.
    if page list is empty or has only one element, return NULL.
    if victim is the last element of the list, move it to the first element */
static struct list_elem *get_next_victim(void){
    struct list_elem *ret;

    if(victim == NULL){
        if(!list_empty(&plist)){
            ret = list_begin(&plist);
            victim = list_entry(ret, struct page, plist_elem);
        }
        else{
            ret = NULL;
        }
    }
    else{
        ret = list_next(&victim->plist_elem);
        if(ret == list_end(&plist)){
            if(list_size(&plist) == 1){
                ret = NULL;
            }
            else{
                ret = list_begin(&plist);
                victim = list_entry(ret, struct page, plist_elem);
            }
        }
        else{
            victim = list_entry(ret, struct page, plist_elem);
        }
    }

    return ret;
}

/*  when not enough physical pages,
    use second chance algorithm to get free pages */
void try_to_free_pages(void){
    struct list_elem *list_elem;
    struct page *victim_page;
    struct thread *victim_thread;

    lock_acquire(&plist_lock);

    while(true){
        list_elem = get_next_victim();

        if(list_elem == NULL){
            lock_release(&plist_lock);
            return;
        }

        victim_page = list_entry(list_elem, struct page, plist_elem);

        victim_thread = victim_page->thread;

        if(pagedir_is_accessed(victim_thread->pagedir, victim_page->vme->vaddr)){
            /* give second chance */
            pagedir_set_accessed(victim_thread->pagedir, victim_page->vme->vaddr, false);
        }
        else{
            /* free page */
            if(pagedir_is_dirty(victim_thread->pagedir, victim_page->vme->vaddr)){
                // should swap out 
                victim_page->vme->type = VM_SWAP;
            }
            if(victim_page->vme->type == VM_SWAP){
                victim_page->vme->swap_slot = swap_out(victim_page->kaddr);
            }
            
            victim_page->vme->is_loaded = false;
            pagedir_clear_page(victim_thread->pagedir, victim_page->vme->vaddr);
            __free_page(victim_page);
            break;
        }
    }


    lock_release(&plist_lock);
}