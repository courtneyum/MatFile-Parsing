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
typedef struct tree_node_ TreeNode;
struct tree_node_
{
	uint64_t address;
	NodeType node_type;
	int node_level;
	uint16_t num_entries;
	uint32_t size;
	uint32_t filter_mask;
	uint64_t* chunk_start;
	uint64_t* chunk_dims;
	TreeNode* parent;
	TreeNode* children;
};

//void fillNode(TreeNode* node, int* start);
void getData(TreeNode* node, Data* object, uint64_t* start, int num_elems);
int insideChunk(uint64_t* chunk_start, uint32_t* chunk_dims, uint64_t* start, uint32_t* dims);
int numChunkElems(uint64_t* chunk_start, uint32_t* chunk_dims, uint32_t* dims);
int fillData(TreeNode* node, Data* object, uint64_t* start, int num_elems);
//int writeToFile(TreeNode* node);
void freeTree(TreeNode* node);

int main()
{
	char* filename = "my_struct1.mat";
	TreeNode root;
	//init maps
	maps[0].used = FALSE;
	maps[1].used = FALSE;

	//init queue
	flushQueue();
	flushHeaderQueue();
	int* num_objects = (int *)malloc(sizeof(int));
	char variable_name[16];
	strcpy(variable_name, "my_struct.array");
	Data* objects = getDataObject("my_struct1.mat", variable_name, num_objects);
	root.address = objects[0].chunk.tree_address;
	printf("Tree address = 0x%lx\n", root.address);
	printf("Chunk dims = [%d, %d, %d]\n", objects[0].chunk.dims[0], objects[0].chunk.dims[1], objects[0].chunk.dims[2]);
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
	uint64_t* start = (uint64_t *)malloc(sizeof(int)*DIMS);
	start[0] = 51;
	start[1] = 50;
	start[2] = 49;
	int num_elems = 100;
	objects[0].double_data = (double *)calloc(num_elems, sizeof(double));
	
	int i;
	root.chunk_start = (uint64_t *)malloc(DIMS*sizeof(uint64_t));
	for (i = 0; i < DIMS; i++)
	{
		root.chunk_start[i] = 0;
	}
	getData(&root, &objects[0], start, num_elems);
	
	close(fd);
	freeTree(&root);
	printf("Complete\n");

}
//dims is the dims of the dataset
void getData(TreeNode* node, Data* object, uint64_t* start, int num_elems)
{
	//static int leaf_counter = 0;
	//static int node_counter = 0;
	/*if (node->address == UNDEF_ADDR)
	{
		return;
	}*/
	//static int curr_elems = 0;
	char* tree_pointer = navigateTo(node->address, 8, TREE);
	if (node->node_level == -1)
	{
		//leaf_counter++;
		//writeToFile(node);
		//return node;
		int err = 0;
		if (insideChunk(node->chunk_start, object->chunk.dims, start, object->dims))
		{
			//there is wanted data in this node.
			err = fillData(node, object, start, num_elems);
			if (err < 0)
			{
				printf("Decompression error: %d\n", err);
				exit(EXIT_FAILURE);
			}
		}
		return;
	}
	//node_counter++;
	node->node_type = getBytesAsNumber(tree_pointer + 4, 1);
	node->node_level = getBytesAsNumber(tree_pointer + 5, 1);
	node->num_entries = getBytesAsNumber(tree_pointer + 6, 2);
	int bytes_needed = 8 + 2*s_block.size_of_offsets + node->num_entries*(KEY_SIZE + s_block.size_of_offsets);
	tree_pointer = navigateTo(node->address, bytes_needed, TREE);

	node->children = (TreeNode *)malloc(node->num_entries*sizeof(TreeNode));
	
	char* key_pointer = tree_pointer + 8 + 2*s_block.size_of_offsets;
	for (int i = 0; i < node->num_entries; i++)
	{
		node->children[i].chunk_start = (uint64_t *)malloc((DIMS + 1)*sizeof(uint64_t));
		node->children[i].size = getBytesAsNumber(key_pointer, 4);
		node->children[i].node_level = node->node_level - 1;
		node->children[i].filter_mask = getBytesAsNumber(key_pointer + 4, 4);
		for (int j = object->num_dims - 1; j >= 0; j--)
		{
			node->children[i].chunk_start[j] = getBytesAsNumber(key_pointer + (DIMS - j)*8, 8);
		}
		node->children[i].address = getBytesAsNumber(key_pointer + (DIMS + 2)*8, s_block.size_of_offsets) + s_block.base_address;
		node->children[i].parent = node;
		//if (insideChunk(node->children[i].chunk_start, object->chunk.dims, start, object->dims))
		if (subToInd(object->dims, node->children[i].chunk_start) <= subToInd(object->dims, start))
		{
			//this condition is only a heuristic for whether we should continue
			//if not in a leaf node it is impossible to know how many elements the node contains
			getData(&node->children[i], object, start, num_elems);
		}
		
		//re-navigate because the memory may have been unmapped in the above call
		tree_pointer = navigateTo(node->address, bytes_needed, TREE);
		key_pointer = (i+1)*(KEY_SIZE + s_block.size_of_offsets) + tree_pointer + 2*s_block.size_of_offsets + 8;
	}
}
int insideChunk(uint64_t* chunk_start, uint32_t* chunk_dims, uint64_t* start, uint32_t* dims)
{
	int num_dims = 0;
	int i = 0;
	while (dims[num_dims] > 0)
	{
		num_dims++;
	}
	int ret = TRUE;
	if (subToInd(dims, chunk_start) <= subToInd(dims, start))
	{
		for (i = 0; i < num_dims; i++)
		{
			if (chunk_start[i] + chunk_dims[i] < start[i])
			{
				ret = FALSE;
				break;
			}
		}
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}
int numChunkElems(uint64_t* chunk_start, uint32_t* chunk_dims, uint32_t* dims)
{
	int num_dims = 0;
	int i = 0;
	int num_chunk_elems = 1;
	while (dims[num_dims] > 0)
	{
		num_dims++;
	}
	for (i = 0; i < num_dims; i++)
	{
		if (chunk_dims[i] + chunk_start[i] <= dims[i])
		{
			num_chunk_elems*=chunk_dims[i];
		}
		else if (chunk_dims[i] + chunk_start[i] > dims[i])
		{
			num_chunk_elems*=(dims[i] - chunk_start[i]);
		}
	}
	return num_chunk_elems;
}
int fillData(TreeNode* node, Data* object, uint64_t* start, int num_elems)
{
	
	char *dataBuffer, *uncompr;
	int curr_chunk_index, chunk_start_index;
	uint64_t index_map[CHUNK_BUFFER_SIZE];
	object->chunk.chunk_size = numChunkElems(node->chunk_start, object->chunk.dims, object->dims);
	chunk_start_index = subToInd(object->dims, node->chunk_start);

	//FILE *uncomprfp;


	/*uncomprfp = fopen(filename, "w");
	if (!uncomprfp)
	{
		perror("Could not open file to write to.");
		exit(1);
	}
	//fwrite(info, strlen(info), 1, uncomprfp);*/


	//read compressed data element into dataBuffer
	dataBuffer = navigateTo(node->address, node->size, 0);
	uint32_t chunk_pos[HDF5_MAX_DIMS + 1] = { 0 };

	uncompr = (char *)malloc(COMP_FACTOR*node->size*sizeof(char));

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

	curr_chunk_index = 0;
	do
	{
		strm.avail_in = node->size;
		strm.next_in = (unsigned char *)dataBuffer;
		
		do
		{
			strm.avail_out = COMP_FACTOR*node->size;
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
			have = COMP_FACTOR*node->size - strm.avail_out;
			
			//copy over data
			memset(chunk_pos, 0 , sizeof(chunk_pos));
			uint8_t curr_max_dim = 2;
			uint64_t db_pos = 0;
			for(uint64_t index = chunk_start_index, anchor = 0; index < object->num_elems && db_pos < object->chunk.chunk_size; anchor = db_pos)
			{
				for (;db_pos < anchor + object->chunk.dims[0] && index < object->num_elems; db_pos++, index++)
				{
					index_map[db_pos] = index;
				}
				chunk_pos[1]++;
				uint8_t use_update = 0;
				for(uint8_t j = 1; j < curr_max_dim; j++)
				{
					if(chunk_pos[j] == object->chunk.dims[j])
					{	
						chunk_pos[j] = 0;
						chunk_pos[j+1]++;
						curr_max_dim = curr_max_dim <= j + 1 ? curr_max_dim + (uint8_t)1 : curr_max_dim;
						use_update++;
					}
				}
				index += object->chunk.chunk_update[use_update];
			}
		
			placeDataWithIndexMap(object, &uncompr[0], db_pos, object->elem_size, index_map);
			curr_chunk_index += have;
			/*if (fwrite(uncompr, 1, have, uncomprfp) != have || ferror(uncomprfp))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}*/
			have = 0;
		} while (strm.avail_out == 0);// && curr_chunk_index <= num_chunk_elems); //don't want to bother decompressing padding
	} while (ret != Z_STREAM_END);

	//fclose(uncomprfp);
	free(uncompr);

	//call_number++;
	return ret;
}
void freeTree(TreeNode* node)
{
	if (node->node_level == -1)
	{
		return;
	}

	for(int i = 0; i < node->num_entries; i++)
	{
		freeTree(&node->children[i]);
	}
	free(node->children);
}