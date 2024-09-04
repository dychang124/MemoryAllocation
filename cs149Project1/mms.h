/* This is the shared header file which the client test program uses to use the mms functions */


#define MMS_SUCCESS             (0)
#define OUT_OF_MEMORY           (100)
#define MEM_TOO_SMALL           (101)
#define INVALID_DEST_ADDR       (102)
#define INVALID_CPY_ADDR        (103)
#define INVALID_MEM_ADDR        (104)
#define MMS_SYS_ERROR           (105)
#define MMS_VALID_ADDR          (106)

#define MAX_BUF_SIZE (1024)
#define MMS_Filesize  (MAX_BUF_SIZE)
// #define MMS_Filesize  (MAX_BUF_SIZE + size_of( struct mem_map_table ) * 20)
#define IPC_ERROR (-1)

char MMS_Filename[] = "./mms_file";

struct mem_map_table_entry
{
    long    pid;
    int 	mem_req_size;
	int     actual_mem_size;
	int		offset;
	char*   buffer_ptr;
    time_t  tstamp; 
};

struct mem_map_table
{
	int mem_size;
	int min_block_size;
	struct mem_map_table_entry mmt_entry[20];
	char mem_start[MAX_BUF_SIZE];
};

// exported functions

extern char* mmc_init ( int mem_size, int boundary_size );
extern char* mms_init ();


/* char* mms_malloc (  int size, int* error_code )

	-  Allocate a piece of memory given the input size.
	-  If successful, returns a valid pointer.  Otherwise it returns NULL (0) and set the error_code.
	-  Possible errors: 100
 */
extern char* mms_malloc(int size,  int* error_code);

/* int mms_memset ( char* dest_ptr, char c, int size )

	-  Set the destination buffer with a character of certain size.
	-  If successful, returns 0.  Otherwise it returns an error code.
	-  Possible errors: 101, 102
 */
extern int mms_memset(char* dest_ptr,  char c,  int size);

/* int mms_memcpy ( char* dest_ptr, char* src_ptr, int size )

	-  Copy the fixed number of bytes from source to destination. 
	-  If successful, returns 0.  Otherwise it returns an error code.
	-  Possible errors: 101, 103
	-  Must allow external buffer to be pass in as dest_ptr.  (read only request)
 */
extern int mms_memcpy(char* dest_ptr,  char* src_ptr,  int size);

/* int mms_print ( char* src_ptr, int size )

	-  Print the number of characters in hex format to STDOUT. 
	-  If size=0, then print until the first hex 0 to STDOUT.
	-  If successful, returns 0.  Otherwise it returns an error code.
	-  Possible errors: 103
	-  Must allow external buffer to be pass in as src_ptr.  (read only request)
 */ 
extern int mms_print(char* src_ptr,  int size);

/* int mms_free ( char* mem_ptr )

	-  Free the allocated memory.
	-  If successful, returns 0.  Otherwise it returns an error code.
	-  Possible errors: 104
 */ 
extern int mms_free(char* mem_ptr);


