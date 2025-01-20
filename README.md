Bobmem memory allocation library

Usage:
    In bash run the command "make" and then "./bobmem" to run the simple showcase "main.c" program.
    Bobmem can be also used in other c programs by for example using the command "gcc program.c bobmem.c"
    and then "./a.out". This requires using the user functions listed below in the program.c and including the
    bobmem header bobmem.h.


User functions:

    void *boballoc(size_t size):
        Returns the starting memory address of size allocated bytes.
        In failure returns NULL.
        Similiar to stdlib malloc for user.
    
    void bobfree(void *address);
        Frees the allocated memory at address pointer.
        It is user responsiblity to ensure the pointer is correct.
        With wrong address the behaviour is undefined.
        Similiar to stdlib free for user.

    void print_memory(void);
        Prints each memory chunk and all the memory blocks inside each chunk.
        Also shows which blocks are free/allocated.

Information about implementation:

    Sbrk System Call:
        When new memory is needed from the Operating System, the sbrk() system call is used.
        The amount of memory requested from the OS is the minimum amount of memory pages needed to fit the user requested size in boballoc().
        For example if user requests 10000 bytes and page size is 4KB then 3 pages of memory are requested from the OS.

    Memory Data Structures:
        
        memchunk:
            Every time the sbrk() system call is called with a positive value, a new chunk is created. 
            Chunks consist of at least one memblock.
            Chunks have metadata that includes:
                -Usable size of the chunk (size excluding metadata)
                -Pointers to the next and previous chunks
                -Pointers to the first and last memblocks inside the chunk

        memblock:
            Memblocks are the structures used to save users data from boballoc().
            Memblocks are stored inside chunks.
            Blocks have metadata that includes:
                -Is the block free
                -Usable size of the block (size excluding metadata)
                -Pointers to the next and previous blocks.


    First-fit technique:
        When user requests memory using boballoc() the program follows the linked list of memchunks
        and the linked lists of memblocks inside the chunks.
        When a free memblock that is large enough to store user requested bytes, it is marked as allocated
        and the address after the metadata (beginning of usable memory) is returned.
        If the allocated block is larger than requested it will be divided into two blocks:
            One allocated large enough for the user data.
            One free with the remaining memory.
        The exception is if the remaining memory would be too small to fit block metadata + at least 1 byte.
        In this case the block is not divided.
        If the last block in the last chunk is reached without finding a suitable block then the sbrk() system call is used
        and a new chunk is created with a single large block that can be divided later.

    Things to note:
        Bobmem uses slightly more memory than the user requests, as chunk/block metadata is needed for the linked lists.
        
        The address that is actually returned to the user does not point to any metadata but the usable memory.
        
        When bobfree() is called the input is the beginning of the usable memory.
        The metadata for the block is reached by moving the memory address:
        metadata = (memblock *)((char *)address-sizeof(memblock));

        Due to using sbrk() for getting memory from the OS, only the last memchunk in the linked list can be removed.
        This is done by using a negative input for sbrk.
        
        More technical information on the implementation can be found in the files bobmem.c and bobmem.h

AI Usage:
    ChatGPT was used for: 
        Planning the implementation:
            For example listing the pros and cons of different system calls (brk, sbrk, mmap)
            Giving ideas how to structure into blocks and chunks
        Finding bugs in the code
        Helping with using correct teminology
        Helping with syntax:
            For example type casting pointers
        Creating the Makefile
