#include "mapping.h"

#define CHUNK 16384

Superblock getSuperblock(int fd, size_t file_size)
{
	char* superblock_pointer = findSuperblock(fd, file_size);
	Superblock s_block = fillSuperblock(superblock_pointer);

	if (maps[0].used)
	{
		maps[0] = unmap(maps[0], "getSuperblock");
	}

	return s_block;
}
char* findSuperblock(int fd, size_t file_size)
{
	char* chunk_start = "";
	size_t page_size;
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	page_size = si.dwPageSize;

	//Assuming that superblock is in first 8 512 byte chunks
	maps[0].map_start = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	maps[0].bytes_mapped = page_size;
	maps[0].offset = 0;
	maps[0].used = TRUE;

	if (maps[0].map_start == NULL || maps[0].map_start == MAP_FAILED)
	{
		printf("mmap() unsuccessful in findSuperblock(), Check errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	chunk_start = maps[0].map_start;
	
	while (strncmp(FORMAT_SIG, chunk_start, 8) != 0 && (chunk_start - maps[0].map_start) < page_size)
	{
		chunk_start += 512;
	}

	if ((chunk_start - maps[0].map_start) >= page_size)
	{
		printf("Couldn't find superblock in first 8 512-byte chunks. I am quitting.\n");
		exit(EXIT_FAILURE);
	}

	return chunk_start;
}

Superblock fillSuperblock(char* superblock_pointer)
{
	Superblock s_block;
	//get stuff from superblock, for now assume consistent versions of stuff
	s_block.size_of_offsets = getBytesAsNumber(superblock_pointer + 13, 1);
	s_block.size_of_lengths = getBytesAsNumber(superblock_pointer + 14, 1);
	s_block.leaf_node_k = getBytesAsNumber(superblock_pointer + 16, 2);
	s_block.internal_node_k = getBytesAsNumber(superblock_pointer + 18, 2);
	s_block.base_address = getBytesAsNumber(superblock_pointer + 24, s_block.size_of_offsets);

	//read scratchpad space
	char* sps_start = superblock_pointer + 80;
	Addr_Pair root_pair;
	root_pair.tree_address = getBytesAsNumber(sps_start, s_block.size_of_offsets) + s_block.base_address;
	root_pair.heap_address = getBytesAsNumber(sps_start + s_block.size_of_offsets, s_block.size_of_offsets) + s_block.base_address;
	s_block.root_tree_address = root_pair.tree_address;
	enqueuePair(root_pair);

	return s_block;
}
char* navigateTo(uint64_t address, size_t bytes_needed, int map_index)
{
	if (!(maps[map_index].used && address >= maps[map_index].offset && address + bytes_needed < maps[map_index].offset + maps[map_index].bytes_mapped))
	{
		//unmap current page if used
		if (maps[map_index].used)
		{
			maps[map_index] = unmap(maps[map_index], "navigateTo");
		}

		size_t alloc_gran;
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		alloc_gran = si.dwAllocationGranularity;

		//map new page at needed location
		maps[map_index].offset = (off_t)(address/alloc_gran)*alloc_gran;
		maps[map_index].bytes_mapped = address - maps[map_index].offset + bytes_needed;
		maps[map_index].map_start = mmap(NULL, maps[map_index].bytes_mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, maps[map_index].offset);
		
		maps[map_index].used = TRUE;
		if (maps[map_index].map_start == NULL || maps[map_index].map_start == MAP_FAILED)
		{
			printf("mmap() unsuccessful in navigateTo(), Check errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}
	return maps[map_index].map_start + address - maps[map_index].offset;
}
char* navigateTo_map(MemMap map, uint64_t address, size_t bytes_needed, int map_index)
{
	if (!(map.used && address >= map.offset && address + bytes_needed < map.offset + map.bytes_mapped))
	{
		//unmap current page if used
		if (map.used)
		{
			map = unmap(map, "navigateTo_map");
		}

		size_t alloc_gran;
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		alloc_gran = si.dwAllocationGranularity;

		//map new page at needed location
		map.offset = (off_t)(address/alloc_gran)*alloc_gran;
		map.bytes_mapped = address - map.offset + bytes_needed;
		map.map_start = mmap(NULL, map.bytes_mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, map.offset);
		
		map.used = TRUE;
		if (map.map_start == NULL || map.map_start == MAP_FAILED)
		{
			printf("mmap() unsuccessful in navigateTo_map(), Check errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}
	return map.map_start + address - map.offset;
}
void readTreeNode(char* tree_pointer, uint32_t size_of_key, uint64_t heap_address, uint64_t parent_address)
{
	Addr_Pair pair;
	int num_dims;
	uint16_t entries_used = 0;
	//uint64_t left_sibling, right_sibling;
	uint8_t node_type = getBytesAsNumber(tree_pointer + 4, 1);
	char* key_pointer, *child_pointer;

	entries_used = getBytesAsNumber(tree_pointer + 6, 2);

	for (int i = 0; i < entries_used; i++)
	{
		if (node_type == 1)
		{
			//raw data chunk node tree
			key_pointer = tree_pointer + 8 + 2*s_block.size_of_offsets + size_of_key*i + s_block.size_of_offsets*i;
			child_pointer = key_pointer + size_of_key;
			pair.chunk.chunk_size = getBytesAsNumber(key_pointer, 4);
			num_dims = size_of_key/8 - 2;
			pair.chunk.chunk_start = malloc((num_dims + 1)*sizeof(uint64_t));
			for (int j = 0; j < num_dims + 1; j++)
			{
				pair.chunk.chunk_start[j] = getBytesAsNumber(key_pointer + (j+1)*8, 8);
				//mexPrintf("chunk_start[%d] = %d\n", j, pair.chunk.chunk_start[j]);
			}

			pair.tree_address = getBytesAsNumber(child_pointer, s_block.size_of_offsets) + s_block.base_address;
			pair.parent_address = parent_address;
			pair.heap_address = 0;
		}
		else if (node_type == 0)
		{
			child_pointer = tree_pointer + 8 + 2*s_block.size_of_offsets + size_of_key*(i+1) + s_block.size_of_offsets*i;
			pair.tree_address = getBytesAsNumber(child_pointer, s_block.size_of_offsets) + s_block.base_address;
			pair.heap_address = heap_address;
			pair.parent_address = parent_address;
		}
		enqueuePair(pair);
	}

}
void readSnod(char* snod_pointer, char* heap_pointer, char* var_name, uint64_t prev_tree_address)
{
	uint16_t num_symbols = getBytesAsNumber(snod_pointer + 6, 2);
	Object* objects = (Object *)malloc(sizeof(Object)*num_symbols);
	uint32_t cache_type;
	Addr_Pair pair;

	//get to entries
	snod_pointer += 8;

	for (int i = 0; i < num_symbols; i++)
	{
		objects[i].this_tree_address = 0;
		objects[i].name_offset = getBytesAsNumber(snod_pointer + SYM_TABLE_ENTRY_SIZE*i, s_block.size_of_offsets);
		objects[i].obj_header_address = getBytesAsNumber(snod_pointer + SYM_TABLE_ENTRY_SIZE*i + s_block.size_of_offsets, s_block.size_of_offsets) + s_block.base_address;
		strcpy_s(objects[i].name, NAME_LENGTH, heap_pointer + 8 + 2*s_block.size_of_lengths + s_block.size_of_offsets + objects[i].name_offset);
		cache_type = getBytesAsNumber(snod_pointer + 2*s_block.size_of_offsets + SYM_TABLE_ENTRY_SIZE*i, 4);
		objects[i].prev_tree_address = prev_tree_address;

		//check if we have found the object we're looking for
		if(var_name != NULL && strcmp(var_name, objects[i].name) == 0)
		{
			flushHeaderQueue();

			//if another tree exists for this object, put it on the queue
			if (cache_type == 1)
			{
				pair.tree_address = getBytesAsNumber(snod_pointer + 2*s_block.size_of_offsets + 8 + SYM_TABLE_ENTRY_SIZE*i, s_block.size_of_offsets) + s_block.base_address;
				pair.heap_address = getBytesAsNumber(snod_pointer + 3*s_block.size_of_offsets + 8 + SYM_TABLE_ENTRY_SIZE*i, s_block.size_of_offsets) + s_block.base_address;
				objects[i].this_tree_address = pair.tree_address;
				flushQueue(GROUP_T, TRUE);
				priorityEnqueuePair(pair);
			}
			enqueueObject(objects[i]);
			break;
		}
		else if (var_name == NULL)
		{
			//if another tree exists for this object, put it on the queue
			if (cache_type == 1)
			{
				pair.tree_address = getBytesAsNumber(snod_pointer + 2*s_block.size_of_offsets + 8 + SYM_TABLE_ENTRY_SIZE*i, s_block.size_of_offsets) + s_block.base_address;
				pair.heap_address = getBytesAsNumber(snod_pointer + 3*s_block.size_of_offsets + 8 + SYM_TABLE_ENTRY_SIZE*i, s_block.size_of_offsets) + s_block.base_address;
				objects[i].this_tree_address = pair.tree_address;
				flushQueue(GROUP_T, TRUE);
				priorityEnqueuePair(pair);
			}
			enqueueObject(objects[i]);
		}
	}

	free(objects);
}
void getDataFromTree(Data* object, uint64_t tree_address)
{
	char* tree_pointer;
	Bytef* uncompressedData;
	size_t default_bytes;
	uint32_t bytes_output = 0;
	int start_index, num_dims;
	uint32_t* dims;
	Addr_Pair root_pair, data_pair;
	uint64_t* indices;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	default_bytes = si.dwPageSize;

	//reverse dims
	num_dims = object->num_dims;
	dims = malloc((num_dims + 1)*sizeof(uint32_t));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
	}
	dims[num_dims] = 0;

	flushQueue(CHUNK_T, TRUE);
	
	root_pair.tree_address = tree_address;
	root_pair.heap_address = 0; //no heap
	enqueuePair(root_pair);

	//search for data chunks
	while (queue.length > 0)
	{
		data_pair = dequeuePair();
		tree_pointer = navigateTo(data_pair.tree_address, default_bytes, TREE);

		if (strncmp("TREE", tree_pointer, 4) == 0)
		{
			readTreeNode(tree_pointer, (object->num_dims + 2)*8, data_pair.heap_address, data_pair.tree_address);
		}
		else
		{
			uncompressedData = decompressChunk(data_pair, &bytes_output);
			//place uncompressed data in the appropriate data field of object

			if (object->char_data != NULL)
			{

			}
			else if (object->double_data != NULL)
			{
				indices = reverseIndices(data_pair.chunk.chunk_start, object->num_dims);
				start_index = subToInd(dims, indices);
				free(indices);
				for (int i = start_index; i < start_index + bytes_output/sizeof(double) && i < object->num_elems; i++)
				{
					object->double_data[i] = convertHexToFloatingPoint(getBytesAsNumber(uncompressedData + (i - start_index)*object->elem_size, object->elem_size));
				}
			}
			else if (object->ushort_data != NULL)
			{

			}
			else
			{
				//
			}
		}
	}
}
Bytef* decompressChunk(Addr_Pair pair, uint32_t* bytes_output)//bytes_output is an out parameter
{
	//IMPROVEMENT
	//By passing in chunk_start and num_elems, we can stop decompressing when we have all the elements, potentially saving time by not decompressing a bunch of zero padding
	int bytesToRead;
	Bytef *dataBuffer, *uncompr, *unusedUncomprPointer, *unusedDataBufferPointer;
	uLongf uncomprLen;
	int numResize = 0;
	int uncomprAvail;

	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	bytesToRead = pair.chunk.chunk_size;	

	dataBuffer = navigateTo(pair.tree_address, bytesToRead, TREE);
	unusedDataBufferPointer = dataBuffer;
	uncomprLen = COMP_FACTOR*bytesToRead;
	uncompr = malloc(uncomprLen*sizeof(char));
	unusedUncomprPointer = uncompr;
	uncomprAvail = uncomprLen;

	/* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
    	if (bytesToRead >= CHUNK)
    	{
    		strm.avail_in = CHUNK;
    		strm.next_in = memmove(in, unusedDataBufferPointer, CHUNK);
    		unusedDataBufferPointer += CHUNK;
    		bytesToRead -= CHUNK;
    	}
    	else
    	{
    		strm.avail_in = bytesToRead;
    		strm.next_in = memmove(in, unusedDataBufferPointer, bytesToRead);
    		bytesToRead = 0;
    	}
        
        if (strm.avail_in == 0)
            break;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                free(dataBuffer);
                return NULL;
            }
            have = CHUNK - strm.avail_out;

            while (uncomprAvail < have)
            {
				//IMPROVEMENT
				//this loop can be eliminated by keeping track of the size of uncompr, and using have to calculate how much larger it needs to be
            	numResize++;
            	uncompr = resize(uncompr, (numResize + 1)*uncomprLen, numResize*uncomprLen); //Can't assume that uncompr points to the same place
            	unusedUncomprPointer = uncompr + (numResize*uncomprLen - uncomprAvail);
            	uncomprAvail = uncomprAvail + uncomprLen;
            }
            unusedUncomprPointer = memmove(unusedUncomprPointer, out, have);
            unusedUncomprPointer += have;
            uncomprAvail -= have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    bytes_output[0] = strm.total_out;
	uncompr = resize(uncompr, bytes_output[0], (numResize + 1)*uncomprLen);
    (void)inflateEnd(&strm);
	
	return uncompr;
}
uint32_t* readDataSpaceMessage(char* msg_pointer, uint16_t msg_size)
{
	//assume version 1 and ignore max dims and permutation indices
	uint8_t num_dims = *(msg_pointer + 1);
	uint32_t* dims = malloc((num_dims + 1)*sizeof(uint32_t));

	for (int i = num_dims - 1; i >= 0; i--)
	{
		dims[i] = getBytesAsNumber(msg_pointer + 8 + i*s_block.size_of_lengths, 4);
	}
	dims[num_dims] = 0;
	return dims;
}
void readDataTypeMessage(Data* object, char* msg_pointer, uint16_t msg_size)
{
	//assume version 1
	uint8_t class = *(msg_pointer) & 7; //only want bottom 4 bits
	uint32_t size = *(msg_pointer + 4);
	object->elem_size = size;
	Datatype type = UNDEF;

	switch(class)
	{
		case 0:
			//fixed point (string)

			switch(size)
			{
				case 1:
					//"char"
					type = CHAR_T;
					break;
				case 2:
					//"uint16_t"
					type = UINTEGER16_T;
					break;
				default:
					type = UNDEF;
					break;
			}

			break;
		case 1:
			//floating point
			//assume double precision
			type = DOUBLE_T;
			break;
		case 7:
			//reference (cell), data consists of addresses aka references
			type = REF_T;
			break;
		default:
			//ignore
			type = UNDEF;
			break;
	}
	object->type = type;

}
