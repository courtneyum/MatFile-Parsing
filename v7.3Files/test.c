
#include "mapping.h"

void cellTest(Data* objects);
void arrayTest(Data* objects);
void integerTest(Data* objects);
void doubleTest(Data* objects);
void stringTest(Data* objects);
int main()
{
	Data* objects;
	char* variable_name = (char *)malloc(50);

	strcpy(variable_name, "string");
	objects = getDataObject("my_struct.mat", variable_name);
	stringTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x288);
	freeDataObjects(objects, 1);
	printf("string test succeeded.\n");

	//dbl
	strcpy(variable_name, "dbl");
	objects = getDataObject("my_struct.mat", variable_name);
	doubleTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x288);
	freeDataObjects(objects, 1);
	printf("dbl test succeeded.\n");

	//integer
	strcpy(variable_name, "integer");
	objects = getDataObject("my_struct.mat", variable_name);
	integerTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x288);
	freeDataObjects(objects, 1);
	printf("integer test succeeded.\n");

	//array
	strcpy(variable_name, "array");
	objects = getDataObject("my_struct.mat", variable_name);
	arrayTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x288);
	freeDataObjects(objects, 1);
	printf("array test succeeded.\n");

	//cell
	strcpy(variable_name, "cell");
	objects = getDataObject("my_struct.mat", variable_name);
	cellTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x288);
	freeDataObjects(objects, 4);
	printf("cell test succeeded.\n");
	
	//my_struct.string
	strcpy(variable_name, "my_struct.string");
	objects = getDataObject("my_struct.mat", variable_name);
	stringTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x1398);
	freeDataObjects(objects, 1);
	printf("my_struct.string test succeeded.\n");

	//my_struct.double
	strcpy(variable_name, "my_struct.double");
	objects = getDataObject("my_struct.mat", variable_name);
	doubleTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x1398);
	freeDataObjects(objects, 1);
	printf("my_struct.double test succeeded.\n");

	//my_struct.integer
	strcpy(variable_name, "my_struct.integer");
	objects = getDataObject("my_struct.mat", variable_name);
	integerTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x1398);
	freeDataObjects(objects, 1);
	printf("my_struct.integer test succeeded.\n");

	//my_struct.array
	strcpy(variable_name, "my_struct.array");
	objects = getDataObject("my_struct.mat", variable_name);
	arrayTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x1398);
	freeDataObjects(objects, 1);
	printf("my_struct.array test succeeded.\n");

	//my_struct.cell
	strcpy(variable_name, "my_struct.cell");
	objects = getDataObject("my_struct.mat", variable_name);
	cellTest(objects);
	assert(objects[0].this_tree_address == 0);
	assert(objects[0].parent_tree_address == 0x1398);
	freeDataObjects(objects, 4);
	printf("my_struct.cell test succeeded.\n");

	//my_struct
	strcpy(variable_name, "my_struct");
	objects = getDataObject("my_struct.mat", variable_name);
	assert(objects[0].this_tree_address == 0x1398);
	assert(objects[0].parent_tree_address == 0x288);
	Data* cell_objects;
	int num_cell = 0;
	for (int i = 0; i < 9; i++)
	{
		if (strcmp(objects[i].name, "integer") == 0)
		{
			integerTest(&objects[i]);
		}
		else if (strcmp(objects[i].name, "dbl") == 0)
		{
			doubleTest(&objects[i]);
		}
		else if (strcmp(objects[i].name, "array") == 0)
		{
			arrayTest(&objects[i]);
		}
		else if (strcmp(objects[i].name, "string") == 0)
		{
			stringTest(&objects[i]);
		}
		else if (strcmp(objects[i].name, "cell") == 0 && objects[i].type == REF)
		{
			cell_objects = (Data *)malloc(4*sizeof(Data));
			cell_objects[num_cell] = objects[i];
			num_cell++;
		}
		else if (strcmp(objects[i].name, "double") == 0)
		{
			doubleTest(&objects[i]);
		}
		else if (strcmp(objects[i].name, "my_struct") == 0)
		{
			//do nothing
		}
		else
		{
			cell_objects[num_cell] = objects[i];
			num_cell++;
		}
	}
	cellTest(cell_objects);
	printf("my_struct test succeeded.\n");
}
void cellTest(Data* objects)
{
	Data data;
	assert(objects[0].type == REF);
	assert(strcmp(objects[0].matlab_class, "cell") == 0);
	assert(objects[0].dims[0] == 3);
	assert(objects[0].dims[1] == 1);
	assert(objects[0].double_data == NULL);
	assert(objects[0].udouble_data != NULL);
	assert(objects[0].ushort_data == NULL);
	assert(objects[0].char_data == NULL);
	assert(strcmp(objects[0].name, "cell") == 0);
	for (int i = 1; i < 4; i++)
	{
		data = objects[i];
		assert(data.type == DOUBLE);
		assert(strcmp(data.matlab_class, "double") == 0);
		assert(data.dims[0] == 1);
		assert(data.dims[1] == 1);
		assert(data.double_data != NULL);
		assert(data.udouble_data == NULL);
		assert(data.ushort_data == NULL);
		assert(data.char_data == NULL);
		assert(strcmp(data.name, objects[0].name) == 0);
	}
	assert(objects[1].double_data[0] == 1);
	assert(objects[2].double_data[0] == 1.1);
	assert(objects[3].double_data[0] == 1.2);
}
void arrayTest(Data* objects)
{
	Data data = objects[0];
	assert(data.type == DOUBLE);
	assert(strcmp(data.matlab_class, "double") == 0);
	assert(data.dims[0] == 3);
	assert(data.dims[1] == 2);
	assert(data.double_data != NULL);
	assert(data.udouble_data == NULL);
	assert(data.ushort_data == NULL);
	assert(data.char_data == NULL);
	assert(strcmp(data.name, "array") == 0);
	for (int i = 0; i < 6; i++)
	{
		assert(data.double_data[i] == 1);
	}
}
void integerTest(Data* objects)
{
	Data data = objects[0];
	assert(data.type == DOUBLE);
	assert(strcmp(data.matlab_class, "double") == 0);
	assert(data.dims[0] == 1);
	assert(data.dims[1] == 1);
	assert(data.double_data != NULL);
	assert(data.udouble_data == NULL);
	assert(data.ushort_data == NULL);
	assert(data.char_data == NULL);
	assert(data.double_data[0] == 1);
	assert(strcmp(data.name, "integer") == 0);
}
void doubleTest(Data* objects)
{
	Data data = objects[0];
	assert(data.type == DOUBLE);
	assert(strcmp(data.matlab_class, "double") == 0);
	assert(data.dims[0] == 1);
	assert(data.dims[1] == 1);
	assert(data.double_data != NULL);
	assert(data.udouble_data == NULL);
	assert(data.ushort_data == NULL);
	assert(data.char_data == NULL);
	assert(data.double_data[0] == 2.2);
}
void stringTest(Data* objects)
{
	char string[9] = "Courtney";
	Data data = objects[0];
	assert(data.type == UINT16_T);
	assert(strcmp(data.matlab_class, "char") == 0);
	assert(data.dims[0] == 8);
	assert(data.dims[1] == 1);
	assert(data.double_data == NULL);
	assert(data.udouble_data == NULL);
	assert(data.ushort_data != NULL);
	assert(data.char_data == NULL);
	assert(strcmp(data.name, "string") == 0);
	for (int i = 0; i < 8; i++)
	{
		assert(data.ushort_data[i] == string[i]);
	}
}