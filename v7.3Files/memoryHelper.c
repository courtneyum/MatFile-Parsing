#include "mapping.h"

void freeDataObjects(Data* objects, int num)
{
	for (int i = 0; i < num; i++)
	{
		if (objects[i].char_data != NULL)
		{
			free(objects[i].char_data);
		}
		else if (objects[i].double_data != NULL)
		{
			free(objects[i].double_data);
		}
		else if (objects[i].udouble_data != NULL)
		{
			free(objects[i].udouble_data);
		}
		else if (objects[i].ushort_data != NULL)
		{
			free(objects[i].ushort_data);
		}
		free(objects[i].dims);
		
		if (objects[i].sub_objects != NULL)
		{
			freeDataObjects(objects[i].sub_objects, objects[i].num_sub_objects);
		}
	}
	free(objects);
	
}
void deepCopyDataObject(Data* dest, Data* src)
{
	//initialize dest
	dest->double_data = NULL;
	dest->udouble_data = NULL;
	dest->char_data = NULL;
	dest->ushort_data = NULL;
	dest->dims = NULL;

	dest->type = src->type;
	strcpy_s(dest->matlab_class, CLASS_LENGTH, src->matlab_class);

	if (src->dims != NULL)
	{
		dest->dims = malloc((src->num_dims + 1)*sizeof(uint32_t));
		for (int i = 0; i < src->num_dims + 1; i++)
		{
			dest->dims[i] = src->dims[i];
		}
	}

	if (src->char_data != NULL)
	{
		dest->char_data = malloc(src->num_elems*sizeof(char));
		memmove(dest->char_data, src->char_data, src->num_elems*sizeof(char));
	}

	if (src->double_data != NULL)
	{
		dest->double_data = malloc(src->num_elems*sizeof(double));
		memmove(dest->double_data, src->double_data, src->num_elems*sizeof(double));
	}

	if (src->udouble_data != NULL)
	{
		dest->udouble_data = malloc(src->num_elems*sizeof(uint64_t));
		memmove(dest->udouble_data, src->udouble_data, src->num_elems*sizeof(uint64_t));
	}

	if (src->ushort_data != NULL)
	{
		dest->ushort_data = malloc(src->num_elems*sizeof(uint16_t));
		memmove(dest->ushort_data, src->ushort_data, src->num_elems*sizeof(uint16_t));
	}

	strcpy_s(dest->name, NAME_LENGTH, src->name);

	dest->this_tree_address = src->this_tree_address;
	dest->parent_tree_address = src->parent_tree_address;

	/*if (dest->sub_objects != NULL)
	{
		free(dest->sub_objects);
	}
	dest->sub_objects = NULL;*/

	dest->num_sub_objects = 0;
	dest->chunk = src->chunk;
	dest->num_dims = src->num_dims;
	dest->num_elems = src->num_elems;
	dest->elem_size = src->elem_size;
}
MemMap unmap(MemMap map, const char callingFunction[])
{
	if (munmap(map.map_start, map.bytes_mapped) != 0)
	{
		printf("munmap() unsuccessful in %s, Check errno: %d\n", callingFunction, errno);
		printf("1st arg: %s\n2nd arg: %lu\nUsed: %d\n", map.map_start, map.bytes_mapped, map.used);
		exit(EXIT_FAILURE);
	}
	map.used = FALSE;
	return map;
}
char* resize(char* data, size_t new_size, size_t old_size)
{
	char* ret = realloc(data, new_size);
	if (ret == NULL)
	{
		ret = calloc(new_size, sizeof(char));
		memmove(ret, data, old_size);
		free(data);
	}
	return ret;
}