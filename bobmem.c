#include "bobmem.h"
#include <unistd.h>
#include <stdio.h>

    static void init() {
        /* System  dependent */
        PAGE_SIZE = getpagesize();
    
        first_chunk = NULL;
        last_chunk = NULL;

        initialized = 1;
        
    }

    void *boballoc(size_t size) {
        void *chunk_result;

        if (size < 1) {
            return NULL;
        }

        if (!initialized) {
            init();
        }

        /*Create first node for linked list of chunks if missing*/
        if (first_chunk == NULL) {

            /* Get first chunk from OS */
            chunk_result = add_chunk(size);  

            /* Failed to allocate chunk*/
            if (chunk_result == NULL) {
                return NULL;
            }

            first_chunk = (memchunk *)chunk_result;
            last_chunk = first_chunk;
        }  

        return (first_fit(size));




    }

    void bobfree(void *address) {
        /*Cannot be called when uninitialized*/
        if (!initialized) return;

        memblock *metadata; 
        metadata = (memblock *)((char *)address-sizeof(memblock));
        metadata->is_free = 1;

        /*If next block free combine to a larger block by extending size of current*/
        if (metadata->next != NULL && metadata->next->is_free) {


            metadata->size += (metadata->next->size+sizeof(memblock)); 

            /*Update linked list */
            if (metadata->next->next != NULL) {
                metadata->next->next->prev = metadata;
            }

            metadata->next = metadata->next->next;
        }

        /*Same for prev block*/
        if (metadata->prev != NULL && metadata->prev->is_free) {
            metadata->prev->size += (metadata->size + sizeof(memblock)); 

            if (metadata->next != NULL) {
                metadata->next->prev = metadata->prev;
            }

            metadata->prev->next = metadata->next;

        }

        /*Checking if last chunks are free and then moving program break*/
        remove_chunks();
    }

    static void *add_chunk(size_t size) {
        void *brk_result;
        memchunk *new_chunk;
        memblock *new_block;
        size_t chunk_size = PAGE_SIZE;

        /*Chunk must have at least space for chunk metadata and one block metadata*/
        while (chunk_size < size+sizeof(memchunk)+sizeof(memblock)) {
            chunk_size += PAGE_SIZE;
        }

        /*Request memory from OS using sbrk system call */
        brk_result = sbrk(chunk_size);

       /*Failed to add new mem*/
        if (brk_result == (void *)-1) {
            return NULL;
        }
        else {
            new_chunk = brk_result;

            /*Init a new block that is the size of the chunks usable memory*/
            new_block = (memblock *)((char *)new_chunk + sizeof(memchunk));
            new_block->prev = NULL;
            new_block->next = NULL;
            new_block->size = chunk_size-sizeof(memchunk)-sizeof(memblock);
            new_block->is_free = 1;

            new_chunk->first = new_block;
            new_chunk->last = new_block;
            new_chunk->size = chunk_size-sizeof(memchunk);
            new_chunk->prev = last_chunk;
            new_chunk->next = NULL;

            if(last_chunk != NULL) {
                last_chunk->next = new_chunk;
            }
            last_chunk = new_chunk;
        }

        return new_chunk;

        
        
    }

    static void add_block(void *ptr, size_t size) {

        size_t usable_mem = size - sizeof(memblock);

        memblock* new_block = (memblock *)ptr;
        new_block->size = usable_mem;
        new_block->next = NULL;
        new_block->prev = NULL;
        new_block->is_free = 1;

    }

    /*Finds first free block large enough for size*/
    static void *first_fit(size_t size) {
        void *new_address;
        memchunk *current_chunk;
        memblock *current_block, *new_block;
        
        current_chunk = first_chunk;
        current_block = current_chunk->first;

        while (1) {
            


             if (current_block->size >= size && current_block->is_free) {
                /*Set block as allocated*/
                current_block->is_free = 0;
                /*Return the address of current block's usable memory
                if memory block is exact the size of requested or if pointless to divide the block.
                */           
                if (current_block->size == size
                    /* No reason to split the memory block to two if the remainder cannot fit
                    metadata + at least one byte*/
                    || (current_block->size - size) < sizeof(memblock) ) return (void *)(current_block+1);

                else {

                    

                    /*Address of memory after required space 
                    Use chars because 1 char is 1 byte */
                    new_address = (void *)((char *)current_block + sizeof(memblock) + size);

                    /*Add a new block after allocated memory and update the size of current block */
                    add_block(new_address, current_block->size-size);
                    current_block->size = size;

                    new_block = (memblock *)new_address;

                    new_block->next = current_block->next;
                    new_block->prev = current_block;

                    if (current_block->next != NULL) {
                        current_block->next->prev = new_block;
                    }
                    current_block->next = new_block;
                    
                    return (void *)(current_block+1);
                }
             }

            /*If current block is too small or allocated*/
             else {

                /*if current block is the chunk's last block move to next chunk
                or create a new chunk if last chunk*/
                if (current_block->next == NULL) {

                    if (current_chunk->next == NULL) {
                        add_chunk(size);

                        current_block = last_chunk->first;

                        if (current_block->size >= size && current_block->is_free) {
                            
                            current_block->is_free = 0;

                            /*Return the address of current block's usable memory
                            if memory block is exact the size of requested or if pointless to divide the block.*/           
                            if (current_block->size == size

                                /* No reason to split the memory block to two if the remainder cannot fit
                                metadata + at least one byte*/
                                || (current_block->size - size) < sizeof(memblock) ) return (void *)(current_block+1);
                  
                        
                            else {
                                /*Address of memory after required space 
                                Use chars because 1 char is 1 byte */
                                new_address = (void *)((char *)current_block + sizeof(memblock) + size);

                                /*Add a new block after allocated memory and update the size of current block */
                                add_block(new_address, current_block->size-size);
                                current_block->size = size;

                                new_block = (memblock *)new_address;

                                new_block->next = NULL;
                                new_block->prev = current_block;
                                current_block->next = new_block;
                                
                                return (void *)(current_block+1);
                            }
                        }
                    }

                    else {
                        current_chunk = current_chunk->next;
                        current_block = current_chunk->first;

                    }
                       
                }

                else {
                    current_block = current_block->next;
                }
                

            }
        }
    }

    /*Called from bobfree() */
    static void remove_chunks(void) {

        /*If only one block and its free*/
        if (last_chunk->first == last_chunk->last && last_chunk->first->is_free) {

            if (last_chunk == first_chunk) {
                sbrk( -(last_chunk->size+sizeof(memchunk)) );
                initialized = 0;
                first_chunk = NULL;
                last_chunk = NULL;
                return;
            }


            memchunk* temp = last_chunk;

            last_chunk = last_chunk->prev;
            last_chunk->next = NULL;

            sbrk( -(temp->size+sizeof(memchunk)) ); 

        
            /*Call recursively to check if new last_block also empty*/
            return remove_chunks();
        }
    }

    /* Helper function for testing */
    void print_memory(void) {
        printf("Printing allocated memory chunks and blocks:\n\n");

        memchunk* current_chunk;
        int chunk_counter;
        memblock* current_block;
        int block_counter;

        if (!initialized) {
            printf("No memory allocated!\n");
            return;
        }

        current_chunk = first_chunk;
        chunk_counter = 0;

        while(1) {

            printf("Chunk %d: %d total usable bytes\n", chunk_counter, (int)current_chunk->size);

            current_block = current_chunk->first;
            block_counter = 0;

            while(1) {

                printf("    Block %d: %d usable bytes ", block_counter, (int)current_block->size);
                
                if(current_block->is_free) printf("(free)\n");

                else printf("(allocated)\n");

                if (current_block->next == NULL) {
                    break;
                }
                else {
                    current_block = current_block->next;
                }

                block_counter++;
            }

            if (current_chunk->next == NULL) {
                break;
            }
            else {
                current_chunk = current_chunk->next;
            }

            chunk_counter++;
        }

    }
   

