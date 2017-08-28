#include "mapping.h"
#include "zlib.h"
#define DIMS 3
#define DEFAULT_BYTES 0x200
#define KEY_SIZE 40
#define COMP_FACTOR 100

typedef enum
{
	GROUP,
	CHUNK
} NodeType;

typedef struct 
{
	uint32_t size;
	uint32_t filter_mask;
	uint64_t chunk_start[DIMS + 1];
} Key;
typedef struct tree_node_ TreeNode;
struct tree_node_
{
	uint64_t address;
	NodeType node_type;
	int node_level;
	uint16_t num_entries;
	Key* keys;
	TreeNode* children;
	uint32_t size;
};

void fillNode(TreeNode* node);
int writeToFile(uint64_t address, uint32_t size);
void freeTree(TreeNode* node);

int main()
{
	char* filename = "my_struct1.mat";
	TreeNode root;
	root.address = 0x15e8;
	//init maps
	maps[0].used = FALSE;
	maps[1].used = FALSE;

	//init queue
	flushQueue();
	flushHeaderQueue();

	//open the file descriptor
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("open() unsuccessful, Check errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	//get file size
	size_t file_size = lseek(fd, 0, SEEK_END);

	//find superblock;

	s_block = getSuperblock(fd, file_size);

	fillNode(&root);
	close(fd);
	freeTree(&root);
	printf("Complete\n");

}
void fillNode(TreeNode* node)
{
	static int leaf_counter = 0;
	static int node_counter = 0;
	if (node->address == UNDEF_ADDR)
	{
		return;
	}
	char* tree_pointer = navigateTo(node->address, 8, TREE);
	if (node->node_level == -1)
	{
		leaf_counter++;
		writeToFile(node->address, node->size);
		return;
	}
	node_counter++;
	node->node_type = getBytesAsNumber(tree_pointer + 4, 1);
	node->node_level = getBytesAsNumber(tree_pointer + 5, 1);
	node->num_entries = getBytesAsNumber(tree_pointer + 6, 2);
	int bytes_needed = 8 + 2*s_block.size_of_offsets + node->num_entries*(KEY_SIZE + s_block.size_of_offsets);
	tree_pointer = navigateTo(node->address, bytes_needed, TREE);

	node->keys = (Key *)malloc(node->num_entries*sizeof(Key));
	node->children = (TreeNode *)malloc(node->num_entries*sizeof(TreeNode));
	
	char* key_pointer = tree_pointer + 8 + 2*s_block.size_of_offsets;
	for (int i = 0; i < node->num_entries; i++)
	{
		node->keys[i].size = getBytesAsNumber(key_pointer, 4);
		node->children[i].size = node->keys[i].size;
		node->children[i].node_level = node->node_level - 1;
		node->keys[i].filter_mask = getBytesAsNumber(key_pointer + 4, 4);
		for (int j = 0; j < DIMS + 1; j++)
		{
			node->keys[i].chunk_start[j] = getBytesAsNumber(key_pointer + (j+1)*8, 8);
		}
		node->children[i].address = getBytesAsNumber(key_pointer + (DIMS + 2)*8, s_block.size_of_offsets) + s_block.base_address;
		fillNode(&node->children[i]);
		//re-navigate because the memory may have been unmapped in the above call
		tree_pointer = navigateTo(node->address, bytes_needed, TREE);
		key_pointer = (i+1)*(KEY_SIZE + s_block.size_of_offsets) + tree_pointer + 2*s_block.size_of_offsets + 8;
	}
}
int writeToFile(uint64_t address, uint32_t size)
{
	static int call_number = 0;

	char filename[51] = "compressData/UncompressedDataElement";
	char spec[10];
	sprintf(spec, "%d", call_number);
	strcat(filename, spec);
	strcat(filename, ".txt");
	
	int bytesToRead;
	char *dataBuffer, *uncompr;

	FILE *uncomprfp;

	uncomprfp = fopen(filename, "w");
	if (!uncomprfp)
	{
		perror("Could not open file to write to.");
		exit(1);
	}

	bytesToRead = size;

	//read compressed data element into dataBuffer
	dataBuffer = navigateTo(address, size, 0);

	uncompr = (char *)malloc(COMP_FACTOR*bytesToRead*sizeof(char));

	int ret;
	unsigned have;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
	{
		printf("ret = %d\n", ret);
		exit(EXIT_FAILURE);
	}

	do
	{
		strm.avail_in = bytesToRead;
		strm.next_in = (unsigned char *)dataBuffer;
		
		do
		{
			strm.avail_out = COMP_FACTOR*bytesToRead;
			strm.next_out = (unsigned char *)uncompr;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = COMP_FACTOR*bytesToRead - strm.avail_out;
			if (fwrite(uncompr, 1, have, uncomprfp) != have || ferror(uncomprfp))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);

	fclose(uncomprfp);
	free(uncompr);

	call_number++;
	return ret;
}
void freeTree(TreeNode* node)
{
	if (node->node_level == -1)
	{
		return;
	}
	free(node->keys);

	for(int i = 0; i < node->num_entries; i++)
	{
		freeTree(&node->children[i]);
	}
	free(node->children);
}