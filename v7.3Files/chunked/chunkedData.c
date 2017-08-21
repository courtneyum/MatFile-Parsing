#include "mapping.h"
#define DIMS 3
#define DEFAULT_BYTES 0x200
#define KEY_SIZE 40

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
};

void fillNode(TreeNode* node);
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
	if (node->address == UNDEF_ADDR)
	{
		return;
	}
	char* tree_pointer = navigateTo(node->address, 8, TREE);
	if (strncmp(tree_pointer, "TREE", 4) != 0)
	{
		return;
	}
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
		node->keys[i].filter_mask = getBytesAsNumber(key_pointer + 4, 4);
		for (int j = 0; j < DIMS + 1; j++)
		{
			node->keys[i].chunk_start[j] = getBytesAsNumber(key_pointer + (j+1)*8, 8);
		}
		node->children[i].address = getBytesAsNumber(key_pointer + (DIMS + 2)*8, s_block.size_of_offsets) + s_block.base_address;
		fillNode(&node->children[i]);
		//re-navigate because the memory may have been unmapped in the above call
		tree_pointer = navigateTo(node->address, bytes_needed, TREE);
		key_pointer += KEY_SIZE + s_block.size_of_offsets;
	}
}
void freeTree(TreeNode* node)
{
	free(node->keys);

	for(int i = 0; i < node->num_entries; i++)
	{
		freeTree(&node->children[i]);
	}
	free(node->children);
}