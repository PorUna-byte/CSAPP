/* 
 * mm-implicit.c -  Simple allocator based on implicit free lists, 
 *                  first fit placement, and boundary tag coalescing. 
 *
 * Each block has header and footer of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap  
 *  -----------------------------------------------------------------   
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mm.h"
#include "memlib.h"


/* Team structure */
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    8    /* overhead of header, footer and succ pointer(bytes) 4 bytes for alignment*/

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)(p))
#define PUT(p, val)  (*(size_t *)(p) = (val))  

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)  
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define PRED(bp)  bp
#define SUCC(bp)  ((char *)(bp)+WSIZE)

#define GET_PRED(bp) GET(PRED(bp))
#define GET_SUCC(bp) GET(SUCC(bp))
/* $end mallocmacros */

/* Global variables */
static char *heap_listp;  /* pointer to first block */  
#define NumofLists 32   //separate lists have 32 lists, each list is a size class:{1}, {2,3}, {4,5,6,7},{8,9,10,...15} ...
/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkblock(void *bp);

char* Separate_lists[NumofLists]={NULL,};  //separate lists for free blocks.
int List_Index(size_t size)
{
    int i=0;
    while(1)
    {
        if(size)
        {
           size>>=1;
           i++;
        }
        else 
           return i-1;
    }
}

void Insert_List(char* bp)
{   
    size_t successor;
    int index=List_Index(GET_SIZE(HDRP(bp)));
    /*
     Insert bp into the front of list
    */
    if(Separate_lists[index]!=NULL)  
    {
        successor=(size_t)Separate_lists[index];
        PUT(PRED(bp),0);
        PUT(SUCC(bp),successor);
        PUT(PRED(successor),(size_t)bp);
    }
    else
    {    
        PUT(PRED(bp),0);
        PUT(SUCC(bp),0);
    }
    Separate_lists[index]=bp;
}
void Delete_List(char* bp)
{   
    size_t pred=GET_PRED(bp);
    size_t succ=GET_SUCC(bp);
    int index=List_Index(GET_SIZE(HDRP(bp)));
    if(pred&&succ) //bp has predecessor and successor
    {
        PUT(PRED(succ),pred);
        PUT(SUCC(pred),succ);
    }
    else if(pred&&!succ)
        PUT(SUCC(pred),0);
    else if(!pred&&succ)
    {
        PUT(PRED(succ),0);
        Separate_lists[index]=(char*)succ;
    }    
    else
        Separate_lists[index]=NULL;
}

static void printblock(void *bp) 
{
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  
    
    if (hsize == 0) {
	printf("%p: EOL\n", bp);
	return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp, 
	   hsize, (halloc ? 'a' : 'f'), 
	   fsize, (falloc ? 'a' : 'f')); 
}

static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
	printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
	printf("Error: header does not match footer\n");
}
/* 
 * mm_checkheap - Check the heap for consistency 
 */
void mm_checkheap(int verbose) 
{
    char *bp = heap_listp;

    if (verbose)
	printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
	printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
	if (verbose) 
	    printblock(bp);
	checkblock(bp);
    }
     
    if (verbose)
	printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
	printf("Bad epilogue header\n");
}
/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void) 
{  
    /* create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == NULL)
	return -1;
    PUT(heap_listp, 0);                        /* alignment padding */
    PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */ 
    PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */ 
    PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));   /* epilogue header */
    heap_listp += DSIZE;
    memset(Separate_lists,0,NumofLists*sizeof(char *));
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;
    return 0;
}
/* $end mminit */

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size) 
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;      

    /* Ignore spurious requests */
    if (size <= 0)
	return NULL;

    /* Adjust block size to include overhead ,successor pointer and alignment reqs. */
    if (size <= DSIZE)
	asize = DSIZE + OVERHEAD;
    else
	asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
	place(bp, asize);
	return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
	return NULL;
    place(bp, asize);
    return bp;
} 
/* $end mmmalloc */

/* 
 * mm_free - Free a block 
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/* $end mmfree */

/*
 * mm_realloc - naive implementation of mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newp;
    size_t copySize,asize;
    void * next_ptr;
    if(ptr==NULL)
       return mm_malloc(size);
    if(size==0)
    {
        free(ptr);
        return NULL;
    }
    copySize = GET_SIZE(HDRP(ptr));
    if (size <= 0)
	return NULL;

    /* Adjust block size to include overhead ,successor pointer and alignment reqs. */
    if (size <= DSIZE)
	asize = DSIZE + OVERHEAD;
    else
	asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);

    if(GET_SIZE(HDRP(ptr))>=asize)  //oldsize is bigger(or equal) than new size, so we don't need to copy data, just resize current block.
    {  
       size_t remain_size=GET_SIZE(HDRP(ptr))-asize;
       if(remain_size>=(DSIZE+OVERHEAD))  //split if remainder would be at least minimum block size
       {   
           PUT(HDRP(ptr),PACK(asize,1));
           PUT(FTRP(ptr),PACK(asize,1));
           next_ptr=NEXT_BLKP(ptr);
           PUT(HDRP(next_ptr),PACK(remain_size,0));
           PUT(FTRP(next_ptr),PACK(remain_size,0));
           coalesce(next_ptr);
       }
    }
    else if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))&&GET_SIZE(HDRP(ptr))+GET_SIZE(HDRP(NEXT_BLKP(ptr)))>=asize)
    //coalesce with next block would satisfy, no need to transfer data.
    {   
        int coale_size=GET_SIZE(HDRP(ptr))+GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        Delete_List(NEXT_BLKP(ptr));
        if(coale_size-asize>=(DSIZE+OVERHEAD))  //split if remainder would be at least minimum block size
        {   
           PUT(HDRP(ptr),PACK(asize,1));
           PUT(FTRP(ptr),PACK(asize,1));
           next_ptr=NEXT_BLKP(ptr);
           PUT(HDRP(next_ptr),PACK(coale_size-asize,0));
           PUT(FTRP(next_ptr),PACK(coale_size-asize,0));
           coalesce(next_ptr);
        }
        else
        {
           PUT(HDRP(ptr),PACK(coale_size,1));
           PUT(FTRP(ptr),PACK(coale_size,1));
        }
    }
    else  //we need search for a bigger block in free lists and copy data as well.
    {
        if ((newp = mm_malloc(size)) == NULL) {
            printf("ERROR: mm_malloc failed in mm_realloc\n");
            exit(1);
        }
        if(size<copySize)
            copySize=size;
         memcpy(newp, ptr,copySize);
         mm_free(ptr);
         return newp;
    }
    return ptr;
}

/* The remaining routines are internal helper routines */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
	
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) 
	return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */
    
    /* Coalesce if the previous block was free */
    return coalesce(bp);
}
/* $end mmextendheap */

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));   
    Delete_List(bp);
    if ((csize - asize) >= (DSIZE + OVERHEAD)) { //split if remainder would be at least minimum block size
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize-asize, 0));
	PUT(FTRP(bp), PACK(csize-asize, 0));
    Insert_List(bp);
    }
    else { 
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static void *find_fit(size_t asize)
{
    /* separate lists first fit search */
    char *bp;
    int index=List_Index(asize);
    bp=Separate_lists[index];

    while(1)
    {   
        while(bp==NULL)
        {
            index++;
            if(index>=32)
            return NULL; /* no fit found*/    
            bp=Separate_lists[index];
        }
        if(GET_SIZE(HDRP(bp))>asize) //This block is big enough 
            return bp;
        else 
            bp=(char *)GET_SUCC(bp); 
    }
}

/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block
 * The following code is formed from CSAPP text book.
 */
static void *coalesce(void *bp) 
{
    size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    if(prev_alloc&&next_alloc)  //previous block and next block are all allocated
    {  
       Insert_List(bp);  
       return bp;
    }
    else if(prev_alloc&&!next_alloc)//coalesce with next block
    {  
       Delete_List(NEXT_BLKP(bp));  //next free block no longer exist in the separate list.
       size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
       PUT(HDRP(bp),PACK(size,0)); //update header
       PUT(FTRP(bp),PACK(size,0)); //update footer
    }
    else if(!prev_alloc&&next_alloc)//coalesce with previous block
    {
       Delete_List(PREV_BLKP(bp));
       size+=GET_SIZE(FTRP(PREV_BLKP(bp)));
       PUT(HDRP(PREV_BLKP(bp)),PACK(size,0)); //update header
       PUT(FTRP(bp),PACK(size,0)); //update footer
       bp=PREV_BLKP(bp);
    } 
    else  //coalesce with previous block & next block
    {  
       Delete_List(PREV_BLKP(bp));
       Delete_List(NEXT_BLKP(bp));
       size+=GET_SIZE(FTRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
       PUT(HDRP(PREV_BLKP(bp)),PACK(size,0)); //update header
       PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
       bp=PREV_BLKP(bp);
    }
    Insert_List(bp);  
    return bp;
}

