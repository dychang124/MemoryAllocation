

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>  // for getpid()
#include "mms.h"

int bound_size = 16;
struct mem_map_table* global_mm_table = NULL;


void log_to_file(const char* operation) {
    FILE* log_file = fopen("mms.log", "a");  
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    char timestamp[15];
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", timeinfo);


    pid_t pid = getpid();

    fprintf(log_file, "%s %s %d %s\n", timestamp, "mms.cpp", (int)pid, operation);

    fclose(log_file);
}
int get_shared_mem(char* filename, int size) 
{
    key_t key;
    key = ftok(filename, 15);
    if (key == IPC_ERROR)
    {
        printf("ftok failed with key = %d\n", key);
        return key;
    }
    return shmget(key, size, 0644 | IPC_CREAT);
}
struct mem_map_table* get_mm_table()
{
    key_t key;
    int shmid;
    struct mem_map_table *mm_table;
    //printf("getstart%p\n", mm_table->mem_start);
    if (global_mm_table != NULL) {
        return global_mm_table;  // Return the existing table
    }

    key = ftok(MMS_Filename, 15);
    if (key == IPC_ERROR) {
        perror("ftok");
        return NULL;
    }

    shmid = shmget(key, MMS_Filesize, 0644);
    if (shmid == IPC_ERROR) {
        perror("shmget");
        return NULL;
    }

    mm_table = (struct mem_map_table*)shmat(shmid, NULL, 0);
    if (mm_table == (struct mem_map_table*)(-1)) {
       // fprintf(stderr, "shmat failed: %s (errno=%d)\n", strerror(errno), errno);
        perror("shmat");
        return NULL;
    }
    global_mm_table = mm_table;
    //printf("getend%p\n", mm_table->mem_start);


    return global_mm_table;
}
void update_mem_map_table(char* ptr, int size) {
    struct mem_map_table* mm_table = get_mm_table();
    if (mm_table == NULL || ptr == NULL || size <= 0) {
        return;  
    }

    time_t current_time;
    time(&current_time);  

    for (int i = 0; i < 20; i++) {
        if (mm_table->mmt_entry[i].buffer_ptr == ptr) {
            mm_table->mmt_entry[i].actual_mem_size = size;
            mm_table->mmt_entry[i].tstamp = current_time;
            return;  
        }
    }

    // If the pointer was not found in the table, you might want to handle this case
    // For example, you can print an error message or take appropriate action based on your application logic.
    printf("Pointer not found in memory mapping table: %p\n", ptr);
}

char* mms_malloc(int size, int* error_code) {
    struct mem_map_table* mm_table = get_mm_table();
    
    //printf("mallocstart%p\n", mm_table->mem_start);

    if (mm_table == NULL) {
        *error_code = MMS_SYS_ERROR;
        return NULL;
    }

    // Check if size exceeds maximum allowable size
    if (size <= 0 || size > MMS_Filesize) {
        *error_code = OUT_OF_MEMORY;
        return NULL;
    }

    int i;
    for (i = 0; i < 20; i++) {
        if (mm_table->mmt_entry[i].buffer_ptr == NULL) {
            if(i == 0)
            {
                mm_table->mmt_entry[i].offset = 0;
            }
            else{
                mm_table->mmt_entry[i].offset = mm_table->mmt_entry[i-1].offset + mm_table->mmt_entry[i-1].actual_mem_size;
            }
            char* end_addr = mm_table->mem_start + MMS_Filesize;
            char* alloc_addr = &(mm_table->mem_start[mm_table->mmt_entry[i].offset]);

            int bound_adjust = bound_size - (mm_table->mmt_entry[i].offset % bound_size);
            if (bound_adjust != bound_size) {
                mm_table->mmt_entry[i].offset += bound_adjust; // Adjust the offset
                alloc_addr += bound_adjust; // Adjust the allocation address
            }

            int aligned_size = size + (bound_size - (size % bound_size));
            //printf("boundsize%d", bound_size);
            //printf("alignedsize%d", aligned_size);
            if (alloc_addr + aligned_size > end_addr) {
                *error_code = OUT_OF_MEMORY;
                return NULL;  // Allocation would exceed segment boundary
            }
            //printf("MALLOC OFFSET%d", mm_table->mmt_entry[i].offset);

            mm_table->mmt_entry[i].buffer_ptr = alloc_addr;
            mm_table->mmt_entry[i].mem_req_size = size;
            mm_table->mmt_entry[i].actual_mem_size = aligned_size;
            mm_table->mmt_entry[i].pid = getpid();
            
            //printf("mallocend%p\n", mm_table->mem_start);

            time(&mm_table->mmt_entry[i].tstamp);
            *error_code = MMS_SUCCESS;

            log_to_file("mms_malloc");
            return alloc_addr;
        }
    }

    *error_code = OUT_OF_MEMORY;  
    return NULL;
}
int find_entry_index(struct mem_map_table* mm_table, char* ptr) {
    if (mm_table == NULL || ptr == NULL) {
        return -1;  
    }

    for (int i = 0; i < 20; i++) {
        if (mm_table->mmt_entry[i].buffer_ptr == ptr) {
            return i;  
        }
    }

    return -1;  
}



int mms_memset(char* dest_ptr, char c, int size) {
    struct mem_map_table* mm_table = get_mm_table();
    if (mm_table == NULL || dest_ptr == NULL || size <= 0) {
        return MEM_TOO_SMALL;
    }

    int entry_index = find_entry_index(mm_table, dest_ptr);
    if (entry_index == -1) {
        return INVALID_DEST_ADDR; 
    }
    

    char* actual_dest_ptr = mm_table->mem_start + mm_table->mmt_entry[entry_index].offset;

    if (actual_dest_ptr < mm_table->mem_start || actual_dest_ptr >= mm_table->mem_start + MMS_Filesize) {
        return INVALID_DEST_ADDR;
    }

    for (int i = 0; i < size; ++i) {
        actual_dest_ptr[i] = c;
    }
    //update_mem_map_table(actual_dest_ptr, size);
    log_to_file("mms_memset");

    return MMS_SUCCESS;
}

int mms_memcpy(char* dest_ptr, char* src_ptr, int size) {
    struct mem_map_table* mm_table = get_mm_table();
    if (mm_table == NULL || size <= 0) {
        return MEM_TOO_SMALL;  // Invalid parameters
    }

    // Check if src_ptr or dest_ptr is within the valid memory range
    int src_index = find_entry_index(mm_table, src_ptr);
    int dest_index = find_entry_index(mm_table, dest_ptr);

    if (src_index == -1 && dest_index == -1) {
        return INVALID_CPY_ADDR;  // Both pointers are invalid
    }

    char* actual_dest_ptr = NULL;
    // Calculate the actual memory location based on offset for destination pointer if it's valid
    if (dest_index != -1) {
        actual_dest_ptr = mm_table->mem_start + mm_table->mmt_entry[dest_index].offset;
    }

    // Perform the copy operation without using memcpy
    for (int i = 0; i < size; i++) {
        if (src_ptr[i] == '\0') {
            break;  // Stop copying if we encounter null termination in src_ptr
        }
        if (actual_dest_ptr != NULL) {
            actual_dest_ptr[i] = src_ptr[i];
        }
    }
    log_to_file("mms_memcpy");

    return MMS_SUCCESS;
}


int mms_print(char* src_ptr, int size) {
   // printf("beginniner\n");
    struct mem_map_table* mm_table = get_mm_table();
    
    if (mm_table == NULL || src_ptr == NULL || size < 0) {
        return INVALID_CPY_ADDR;
    }
   // printf("start%p\n", mm_table->mem_start);
   // printf("end%p\n", mm_table->mem_start + MMS_Filesize);


    if (src_ptr < mm_table->mem_start || src_ptr >= mm_table->mem_start + MMS_Filesize) {
        printf("here\n");
        return INVALID_CPY_ADDR;
    }
    //printf("end\n");

    for (int i = 0; i < size || (size == 0 && src_ptr[i] != '\0'); i++) {
        printf("%02X ", src_ptr[i]);  // Assuming ASCII characters
    }
    printf("\n");
    log_to_file("mms_print");

    return MMS_SUCCESS;
}

int mms_free(char* mem_ptr) {
    struct mem_map_table* mm_table = get_mm_table();
    if (mm_table == NULL || mem_ptr == NULL) {
        return INVALID_MEM_ADDR;
    }

    if (mem_ptr < mm_table->mem_start || mem_ptr >= mm_table->mem_start + MMS_Filesize) {
        return INVALID_MEM_ADDR;
    }

    for (int i = 0; i < 20; i++) {
        if (mm_table->mmt_entry[i].buffer_ptr == mem_ptr) {
            mm_table->mmt_entry[i].buffer_ptr = NULL;
            mm_table->mmt_entry[i].pid = 0;
            mm_table->mmt_entry[i].mem_req_size = 0;
            mm_table->mmt_entry[i].actual_mem_size = 0;
            

            log_to_file("mms_free");
            return MMS_SUCCESS;
        }
    }

    return INVALID_MEM_ADDR;
}

char* mmc_init ( int mem_size, int boundary_size )
{
    bound_size = boundary_size;
    char* mms_mem;
    int shared_mem_id = get_shared_mem((char*)MMS_Filename, MMS_Filesize);
    struct mem_map_table* mm_table = get_mm_table();

    if (shared_mem_id == IPC_ERROR)
        return NULL;

    // map the shared mem and return the pointer to the beginning of it
    mms_mem = (char*) shmat( shared_mem_id, NULL, 0);
    if (mms_mem == (char*)IPC_ERROR)
        return NULL;

    for (int i = 0; i < MMS_Filesize; ++i) {
        mms_mem[i] = 0;
    }
    /*int offset = 0;

    for (int i = 0; i < 20; i++) {
        mm_table->mmt_entry[i].buffer_ptr = NULL;
        mm_table->mmt_entry[i].mem_req_size = 0;
        mm_table->mmt_entry[i].actual_mem_size = 0;
        mm_table->mmt_entry[i].pid = 0;
        mm_table->mmt_entry[i].tstamp = 0;
        mm_table->mmt_entry[i].offset = offset;
        offset += boundary_size;  // Increment offset for next entry
    }*/
    return mms_mem;
}

// Use by client program to initiate & attach to shared memory before any mms_* function calls
char* mms_init ()
{
    char* mms_mem;
    int shared_mem_id = get_shared_mem((char*)MMS_Filename, MMS_Filesize);
    struct mem_map_table* mm_table = get_mm_table();

    if (shared_mem_id == IPC_ERROR)
        return NULL;

    // map the shared mem and return the pointer to the beginning of it
    mms_mem = (char*) shmat( shared_mem_id, NULL, 0);
    if (mms_mem == (char*)IPC_ERROR)
        return NULL;


    return mms_mem;
}

/* int mms_memcpy ( char* dest_ptr, char* src_ptr, int size )

	-  Copy the fixed number of bytes from source to destination. 
	-  If successful, returns 0.  Otherwise it returns an error code.
	-  Possible errors: 101, 103
	-  Must allow external buffer to be pass in as dest_ptr.  (read only request)
 */
/*int mms_memcpy(char* dest_ptr,  char* src_ptr,  int size)
{ 
    // Simple copy without checking pointer or boundary
    int rc=0;
    strncpy(dest_ptr, src_ptr, size);
    return rc;
}*/