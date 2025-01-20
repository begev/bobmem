#include "bobmem.h"
#include <stdio.h>



int main(void) {
   
    printf("This is a short demonstration on bobmem memory allocation\n\n");

    printf("Press enter to continue\n");
    while (getchar() != '\n');

    size_t size1 = 20;
    size_t size2 = 500;
    size_t size3 = 8000;
    size_t size4 = 1000;
   
    printf("Allocating memory for byte sizes %d, %d and %d\n\n", size1, size2, size3);

    void *ptr1 = boballoc(size1);
    void *ptr2 = boballoc(size2);
    void *ptr3 = boballoc(size3);


    print_memory();
    
    printf("\nBobmem uses first-fit technique to allocate memory blocks.\n");
    printf("If no suitable block is found a new memory chunk is created in increments of page sizes.\n");
    printf("Typical page size is 4096 bytes.\n");
    printf("When boballoc is called first for %d bytes a chunk is created of the size of one page.\n", size1);
    printf("This chunk also fits the second allocation for %d bytes\n", size2);
    printf("The third allocation for %d bytes is too large to fit in the same chunk, so a new chunk is created.\n", size3);
    printf("The new chunk is 2 pages or 8KB, because a single page chunk wouldnt fit %d bytes.\n\n", size3);

    printf("Press enter to continue\n");
    while (getchar() != '\n');

    printf("Freeing the block with %d bytes\n\n",size3);

    bobfree(ptr3);

    print_memory();

    printf("\nThe second chunk is freed because the only allocated block inside it was freed.\n");
    printf("Only the last chunk can be freed because bobmem uses sbkr() system calls for receiving memory from the OS\n\n");
    
    printf("Press enter to continue\n");
    while (getchar() != '\n');

    printf("Allocating new block of %d bytes\n\n", size4);
    

    ptr3 = boballoc(size4);

    print_memory();

    printf("\nHere the newly allocated %d bytes fit inside the already existing chunk.\n", size4);
    printf("Note that the chunk usable bytes is not the same as page size because of chunk metadata\n");
    printf("Similiarly the sum of the blocks' usable bytes is not equal to chunk's usable bytes due to block metadatas.\n\n");

    printf("Press enter to continue\n");
    while (getchar() != '\n');

    printf("Freeing all allocated blocks\n\n"); 

    bobfree(ptr1);
    bobfree(ptr2);
    bobfree(ptr3);

    print_memory();

    
}
