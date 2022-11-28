#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "lib/kernel/list.h"

static struct list_elem *get_next_victim(void);

/* init page list */
void plist_init(void){
    list_init(&plist);
    lock_init(&plist_lock);
    victim = NULL;
}

/* add page to page list */
void add_page_to_plist(struct page *page){
    lock_acquire(&plist_lock);
    list_push_back(&plist, &page->plist_elem);
    lock_release(&plist_lock);
}

/* delete page from page list */
void del_page_from_plist(struct page *page){
    lock_acquire(&plist_lock);
    list_remove(&page->plist_elem);
    lock_release(&plist_lock);
}

/* allocate a page by given flags*/
struct page *alloc_page(enum palloc_flags flags){
    struct page *page = (struct page *)malloc(sizeof(struct page));
    
    page->kaddr = palloc_get_page(flags);
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
            break;
        }
    }

    lock_release(&plist_lock);

    if(e != list_end(&plist)){
        __free_page(list_entry(e, struct page, plist_elem));
    }
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
    }

    return ret;
}

/*  when not enough physical pages,
    use second chance algorithm to get free pages */
void try_to_free_pages(enum palloc_flags flags){

    lock_acquire(&plist_lock);

    

    lock_release(&plist_lock);
}