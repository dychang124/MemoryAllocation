// mms_test2.cpp

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
    char test_str1[20], test_str2[20], test_str3[20], test_str4[20];
	
    p_mem_size = boundary_size = 0;
	
	if (argc != 3)
	{
		printf("Usage: %s Test_string1 Test_string2\n", argv[0]);
		return 1;
	}

    strcpy(test_str1, argv[1]);
    strcpy(test_str2, argv[2]);

    ts = time(NULL);
	// Print the current PID, Time
    printf("\nCurrent PID = %ld, Current Time = %s\n", (long)getpid(), ctime(&ts));
   
    mem_ptr = mms_init();
    if (mem_ptr == NULL)
    {
        printf("mmc_init error\n");
        return -1;
    }

    // mem_ptr and its attributes should only be used inside mms.cpp but use it here as example
    mm_table = (struct mem_map_table*)mem_ptr;

    printf("MMS_Filesize = %d; Boundary_size = %d; \n", mm_table->mem_size, mm_table->min_block_size);

    printf("mem_start = %s\n", mm_table->mem_start);

// print the string where mms_test1 program stored.
    printf("mem_start+20 = %s\n", (char*)(mm_table->mem_start+32));

//    rc = mms_memcpy((char*)(mm_table->mem_start+32), (char*)test_str1, 20);

    return rc;
}



	


