#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>  // for getpid()
#include "mms.h"


void dump_memory_hex(char* mem_ptr, int size) {
    for (int i = 0; i < size; i++) {
        if (i % 32 == 0) {
            printf("\n%p  ", &mem_ptr[i]);
        }
        printf("%01X ", (unsigned char)mem_ptr[i]);
        
    }
    printf("\n");
}
void print_memory_mapping_table(struct mem_map_table *mm_table) {
    printf("Memory Mapping Table:\n");
    printf("PID\tRequest Size\tActual Size\tClient Address\tLast Reference Time\n");

    for (int i = 0; i < 20; i++) {
        printf("%ld\t%d\t\t%d\t\t%p\t%s", 
            mm_table->mmt_entry[i].pid,
            mm_table->mmt_entry[i].mem_req_size,
            mm_table->mmt_entry[i].actual_mem_size,
            mm_table->mmt_entry[i].buffer_ptr,
                ctime(&mm_table->mmt_entry[i].tstamp)); // Convert timestamp to readable format
    }
}

int main(int argc, char** argv)
{
	int p_mem_size, boundary_size;
	FILE *fptr;
	long pid;
	int i = 0, rc = 0;
    time_t ts;
    char* mem_ptr;
	struct mem_map_table *mm_table;
    char ch;
	
    p_mem_size = boundary_size = 0;
	
	if (argc != 3)
	{
		printf("Usage: %s physical_memory_size boundry_size\n", argv[0]);
		return 1;
	}

	p_mem_size = atoi(argv[1]);
	boundary_size = atoi (argv[2]);
    ts = time(NULL);
	// Print the current PID, Time
    printf("\nCurrent PID = %ld, Current Time = %s\n", (long)getpid(), ctime(&ts));
    printf("\nPhysical Memory Size = %d, Memory Boundary Size = %d\n", p_mem_size, boundary_size);

    mem_ptr = mmc_init ( p_mem_size, boundary_size );
    if (mem_ptr == NULL)
    {
        printf("mmc_init error\n");
        return -1;
    }

    mm_table = (struct mem_map_table*)mem_ptr;
    mm_table->mem_size = MMS_Filesize;
    mm_table->min_block_size = boundary_size;

    //strcpy(mm_table->mem_start, "Hello World");

    printf("MMC functions: D, M, or E\n");
    while ((ch = getchar()) != 'E') {
        if (ch == 'D') {
            printf("Virtual:\n");
            dump_memory_hex(mm_table->mem_start, MMS_Filesize);
        }
        else if (ch == 'M') {
            print_memory_mapping_table(mm_table);
        }
    }

  return rc;
}



	


