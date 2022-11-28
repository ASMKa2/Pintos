#include "vm/swap.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "lib/kernel/bitmap.h"
#include "threads/synch.h"
#include "devices/block.h"

/* 2^12 bytes */
#define PAGE_SIZE 4096

static const int SECTOR_PER_PAGE  = PAGE_SIZE / BLOCK_SECTOR_SIZE;

struct lock swap_lock;
struct block *swap_block;
struct bitmap *swap_bitmap;

void swap_init(size_t used_index, void *kaddr){
    lock_init(&swap_lock);
    swap_block = block_get_role(BLOCK_SWAP);
    swap_bitmap = bitmap_create(block_size(swap_block) / SECTOR_PER_PAGE);
    bitmap_set_all(swap_bitmap, false);
}

void swap_in(size_t used_index, void *kaddr){
    lock_acquire(&swap_lock);

    if(bitmap_test(swap_bitmap, used_index)){
        for(int i = 0; i < SECTOR_PER_PAGE; i++){
            block_read(swap_block, used_index * SECTOR_PER_PAGE + i, kaddr + i * BLOCK_SECTOR_SIZE);
        }
    }
    
    lock_release(&swap_lock);
}

size_t swap_out(void *kaddr){
    lock_acquire(&swap_lock);

    size_t free_index = bitmap_scan_and_flip(swap_bitmap, 0, 1, true);

    if(free_index != BITMAP_ERROR){
        for(int i = 0; i < SECTOR_PER_PAGE; i++){
            block_write(swap_block, free_index * SECTOR_PER_PAGE + i, kaddr + i * BLOCK_SECTOR_SIZE);
        }
    }

    lock_release(&swap_lock);
    
    return free_index;
}