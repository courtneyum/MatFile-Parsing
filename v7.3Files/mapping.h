#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0
#define FORMAT_SIG "\211HDF\r\n\032\n"
#define MAX_Q_LENGTH 20
#define TREE 0
#define HEAP 1
#define UNDEF_ADDR 0xffffffffffffffff
#define SYM_TABLE_ENTRY_SIZE 40
#define MAX_OBJS 100
#define CLASS_LENGTH 20
#define NAME_LENGTH 30
#define MAX_SUB_OBJECTS 30
#define USE_SUPER_OBJECT_CELL 1
#define USE_SUPER_OBJECT_ALL 2
#define HDF5_MAX_DIMS 32
#define CHUNK_BUFFER_SIZE 1048576 /*1MB size of the buffer used in zlib*/

typedef struct
{
	uint64_t tree_address;
	uint64_t heap_address;
} Addr_Pair;

typedef struct
{
	Addr_Pair pairs[MAX_Q_LENGTH];
	int front;
	int back;
	int length;
} Addr_Q;

typedef struct 
{
	uint8_t size_of_offsets;
	uint8_t size_of_lengths;
	uint16_t leaf_node_k;
	uint16_t internal_node_k;
	uint64_t base_address;
	uint64_t root_tree_address;
} Superblock;

typedef struct 
{
	char* map_start;
	uint64_t bytes_mapped;
	uint64_t offset;
	int used;
} MemMap;

typedef struct
{
	uint64_t name_offset;
	char name[NAME_LENGTH];
	uint64_t obj_header_address;
	uint64_t prev_tree_address;
	uint64_t this_tree_address;
} Object;

typedef struct
{
	Object objects[MAX_Q_LENGTH];
	int front;
	int back;
	int length;
} Header_Q;

typedef enum
{
	UNDEF,
	CHAR,
	DOUBLE,
	UINT16_T,
	REF,
	STRUCT
} Datatype;
typedef struct 
{
	uint8_t num_dims;
	uint64_t tree_address;
	uint32_t* dims;
	uint32_t elem_size;
	uint64_t chunk_update[HDF5_MAX_DIMS];
	uint32_t chunk_size;
}Chunk;

typedef struct data_ Data;
struct data_
{
	Datatype type;
	char matlab_class[CLASS_LENGTH];
	uint32_t* dims;
	char* char_data;
	double* double_data;
	uint64_t* udouble_data;
	uint16_t* ushort_data;
	char name[NAME_LENGTH];
	uint64_t this_tree_address;
	uint64_t parent_tree_address;
	Data* sub_objects;
	Chunk chunk;
	int num_dims;
	int num_elems;
	uint32_t elem_size;
} ;


Superblock getSuperblock(int fd, size_t file_size);
char* findSuperblock(int fd, size_t file_size);
Superblock fillSuperblock(char* superblock_pointer);
char* navigateTo(uint64_t address, uint64_t bytes_needed, int map_index);
char* navigateTo_map(MemMap map, uint64_t address, uint64_t bytes_needed, int map_index);
void readTreeNode(char* tree_address);
void readSnod(char* snod_pointer, char* heap_pointer, char* var_name, uint64_t prev_tree_address);
uint32_t* readDataSpaceMessage(char* msg_pointer, uint16_t msg_size);
void readDataTypeMessage(Data* object, char* msg_pointer, uint16_t msg_size);
void freeDataObjects(Data* objects, int num);
//void placeDataWithIndexMap(Data* object, char* data_pointer, uint64_t num_elems, size_t elem_size, ByteOrder data_byte_order, const uint64_t* index_map);
void placeDataWithIndexMap(Data* object, char* data_pointer, uint64_t num_elems, size_t elem_size, const uint64_t* index_map);

double convertHexToFloatingPoint(uint64_t hex);
int roundUp(int numToRound);
uint64_t getBytesAsNumber(char* chunk_start, int num_bytes);
void indToSub(int index, uint32_t* dims, uint64_t* indices);
int subToInd(uint32_t* dims, uint64_t* indices);
void reverseBytes(char* data_pointer, size_t num_elems);

void enqueuePair(Addr_Pair pair);
void flushQueue();
Addr_Pair dequeuePair();
void priorityEnqueuePair(Addr_Pair pair);
void flushHeaderQueue();
Object dequeueObject();
void priorityEnqueueObject(Object obj);
void enqueueObject(Object obj);

Data* getDataObject(char* filename, char variable_name[], int* num_objs);
void findHeaderAddress(char* filename, char variable_name[]);
void collectMetaData(Data* object, uint64_t header_address, char* header_pointer);
Data* organizeObjects(Data* objects, int num_objects);
//void deepCopy(Data* dest, Data* source);

MemMap maps[2];
Addr_Q queue;
Header_Q header_queue;

int fd;
Superblock s_block;
int is_string;
uint64_t default_bytes;