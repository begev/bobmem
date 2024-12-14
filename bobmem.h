#ifndef BOBMEM_H
#define BOBMEM_H

#include <stddef.h>
#include <stdio.h>

    /* 
    *Fixed size unit for memory management in bytes
    *Depends on the system architecture
    *Defined in init() 
    */
    static size_t PAGE_SIZE;

    /* Has program been initialized */
    static int initialized;



    /* Metadata for blocks*/
    struct  memblock {
        int is_free;

        /*USABLE size not including metadata*/
        size_t size;
        struct memblock* next;
        struct memblock* prev;
    };

    typedef struct memblock memblock;

    /*Metadata for chunks*/
    struct memchunk {
      /*Usable size not including chunk metadata
      At least one block metadata needed*/
      size_t size;
      struct memchunk* next;
      struct memchunk* prev;
      
      struct memblock* first;
      struct memblock* last;
    };

    typedef struct memchunk memchunk;

    /*First and last memchunks in the linked list of chunks. */
    static memchunk *first_chunk;
    static memchunk *last_chunk;

    /*
    *Init function for getting page_size and setting first and last chunk as NULL;
    */
    static void init(void);

    /* 
    *Return memory address that has size bytes of allocated memory.
    *
    *Allocates space for size + metadata size.
    *
    *@param size The size to be allocated
    *@return void pointer to the allocated memory (after metadata)
    */
    void *boballoc(size_t size);

    /*
    *Mark allocated memblock as free.
    *
    *If previous/next block from freed is also free,
    *then combines into a larger block.
    *
    *User responsibility to not try to free unallocated address
    * 
    *@param address The address of allocated memory to free
    */
    void bobfree(void *address);

    /*
    *Adds a new memory chunk to the program in size of pages
    *
    *@param size Required amount of usable bytes
    *@return The address of the of the new memchunk or NULL if failed 
    */
    static void *add_chunk(size_t size);

    /*
    *Adds a memblock starting from ptr with sizeof size (size including metadata)
    *
    *Also adds one large block of memory to the chunk that can be divided later
    *
    *No return value needed when first param is the pointer to the new block
    *
    *@param ptr The pointer to the new memblock
    *@param size How much memory the block has in total (including metadata)
    */
    static void add_block(void *ptr, size_t size);


    /*
    *Follows the linked list for chunks and their blocks
    *to find the first instance of a large enough free block
    *or creates a new chunk if no suitable block is found.
    *
    *Divides the block into two if necessary to not waste memory.
    *
    *@param size The requested amount of usable memory
    *@return Null pointer to the usable part of the allocated block
    */
    static void *first_fit(size_t size);

    /*
    *Removes last memchunk(s) if they are empty.
    *
    *Due to sbrk() limitations can only remove the last chunk,
    *which may leave whole chunks empty without being freed until
    *later chunks are also freed
    *
    *If all chunks are removed set the static var initialized as 0
    */
    static void remove_chunks(void);


    /*
    *Tester function for printing each chunk and the blocks in them.
    *
    *Prints also the amount of usable memory in each chunk/block.
    */
    void print_memory(void);



#endif
