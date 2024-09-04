// mms_test1.cpp

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

    int error_code;

    char* buffer = mms_malloc(20, &error_code);

    
   // rc = mms_memcpy((char*)(mm_table->mem_start+32), (char*)test_str1, 20);
    //rc = mms_memcpy(buffer, (char*)test_str1, 20);
    int result = mms_memset(buffer, 'A', 20);
    if (result != MMS_SUCCESS) {
        printf("Error setting memory: %d\n", result);
        return 1;
    }

    

    if (rc != 0) {
    printf("mms_memcpy error: %d\n", rc);
    // Handle the error if necessary
} else {
    printf("After memcpy, mem_start = %s\n", mm_table->mem_start);
}
    buffer = mms_malloc(50, &error_code);
   // printf("before print%p\n", (void*)buffer);
    rc = mms_memcpy(buffer, (char*)test_str2, 50);
   // printf("print%p\n", (void*)buffer);
    mms_print(buffer,50);
    


    buffer = mms_malloc(20, &error_code);
    //printf("before%p", (void*)buffer);
    rc = mms_memcpy(buffer, (char*)test_str1, 20);
   // printf("after%p", (void*)buffer);
    char* buffer2 = mms_malloc(20, &error_code);
    result = mms_memcpy(buffer2, buffer, 20);
    if (result != MMS_SUCCESS) {
        printf("Error freeing memory: %d\n", result);
        return 1;
    }

    return rc;
}



	


