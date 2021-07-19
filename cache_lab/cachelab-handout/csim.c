#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

//#define DEBUG_ON 
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
   LRU is a counter used to implement LRU replacement policy  */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;

/* The cache we are simulating */
cache_t cache;  
mem_addr_t set_index_mask;
void oom()
{
    printf("out of memory\n");
    exit(-1);
}
/* 
 * initCache - Allocate memory, write 0's for valid and tag and LRU
 * also computes the set_index_mask
 */
void initCache()
{   
    cache=(cache_t)malloc(sizeof(cache_set_t)*S); //allocate space for pointers to each cache set.
    if(cache==NULL)
      oom();

    for(int i=0;i<S;i++) //allocate space for each cache set
    {
    cache[i]=(cache_set_t)malloc(sizeof(cache_line_t)*E); 
    if(cache[i]==NULL)
       oom();
    memset(cache[i],0,sizeof(cache_line_t)*E);   
    }

    set_index_mask=1;
    for(int i=0;i<s-1;i++) //compute set_index_mask as:0000..1111..000 that is the midian s bits is all 1's for set index and others is all 0's .
      set_index_mask=(set_index_mask<<1)+1;
    set_index_mask<<=b;  //note that there are b bits block offset on the right.
    return ;
}


/* 
 * freeCache - free allocated memory
 */
void freeCache()
{
  for(int i=0;i<S;i++)  
  free(cache[i]);

  free(cache);
  return ;
}


/* 
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increast hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 */
void accessData(mem_addr_t addr)
{
    mem_addr_t set_index=(addr&set_index_mask)>>b;
    mem_addr_t request_tag=addr>>(s+b);
    cache_set_t cur_set=cache[set_index];
    for(int i=0;i<E;i++) //search all E lines for matching line.
    {
       if(!(cur_set[i].valid)||cur_set[i].tag!=request_tag)//this line is not valid or doesn't match
          continue;

        //Here the line is valid and matches the tag, which means a hit .
        hit_count++;
        if(verbosity)
           printf("hit\t");
        cur_set[i].lru=lru_counter;//update the lru of the hitting line
        lru_counter++; //advance time
        return;

    }
    //Here it is not in cache
    miss_count++;
    if(verbosity)
        printf("miss\t");    
    int found=0;
    int k=0;
    for(;k<E;k++)
    {
      if(!(cur_set[k].valid)) //we find an empty line
      {
          found=1;
          break;
      }
    }
    if(!found)  //we need envict a existing line using lru strategy.
    { 
        eviction_count++;
        if(verbosity)
            printf("eviction\t");    
        k=-1;
        mem_addr_t farest=0;
        for(int i=0;i<E;i++)  //search all lines for farest line.
        {
           if(lru_counter-cur_set[i].lru>farest)//find the "least recently used" cache line that is:lru is farest from lru_counter
           {
              farest=lru_counter-cur_set[i].lru;
              k=i;
           }
        }
    }
    cur_set[k].valid=1; //set the valid bit,tag and lru
    cur_set[k].tag=request_tag;
    cur_set[k].lru=lru_counter; 
    lru_counter++;  //advance the time
    if(verbosity)  //show the set index and tag
    {
      printf("set:%llu\ttag:%llu",set_index,request_tag);
    }
    return ;
}


/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);
      
            if(verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            accessData(addr);

            /* If the instruction is R/W then access again */
            if(buf[1]=='M')
                accessData(addr);
            
            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine 
 */
int main(int argc, char* argv[])
{
    char c;

    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    /* Compute S, E and B from command line args */
    S=1<<s;
    B=1<<b;
    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif
 
    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}