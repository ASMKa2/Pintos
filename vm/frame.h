#include "vm/page.h"
#include "threads/palloc.h"

void plist_init(void);
void add_page_to_plist(struct page *);
void del_page_from_plist(struct page *);
struct page *alloc_page(enum palloc_flags);
void free_page(void *);
void __free_page(struct page *);

void try_to_free_pages(enum palloc_flags);