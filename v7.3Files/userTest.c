#include "mapping.h"

void printDouble(Data* object);
void printShort(Data* object);
void printCell(Data* object);
void printStruct(Data* object);
void printChar(Data* object);

#define NUM_FILES 5
#define SINGLE_TEST 0
#define ALL_TEST 1
#define MAX_NAME_SIZE 100

//param 1: filename
//param 2: name of variable to retrieve from file
int main (int argc, char* argv[])
{
	char filename[MAX_NAME_SIZE];
	char variable_name[MAX_NAME_SIZE];
	char** filenames;
	char*** variable_names;
	int* num_vars;
	int test_type = ALL_TEST;
	int num_files;

	if (test_type == ALL_TEST)
	{
		//redirect stdout
		fclose(stdout);
		stdout = fopen("log.txt", "w");
		num_files = NUM_FILES;
		//define test cases
		filenames = (char **)malloc(sizeof(char*)*NUM_FILES);
		filenames[0] = "my_struct.mat";
		filenames[1] = "my_struct1.mat";
		filenames[2] = "my_struct2.mat";
		filenames[3] = "my_struct3.mat";
		filenames[4] = "my_struct4.mat";

		num_vars = (int *)malloc(sizeof(int)*num_files);
		num_vars[0] = 11;
		num_vars[1] = 7;
		num_vars[2] = 2;
		num_vars[3] = 4;
		num_vars[4] = 4;

		variable_names = (char ***)malloc(sizeof(char **)*NUM_FILES);
		for (int i =0; i < NUM_FILES; i++)
		{
			variable_names[i] = (char **)malloc(sizeof(char *)*num_vars[i]);
		}

		variable_names[0][0] = "dbl";
		variable_names[0][1] = "array";
		variable_names[0][2] = "string";
		variable_names[0][3] = "cell";
		variable_names[0][4] = "integer";
		variable_names[0][5] = "my_struct";
		variable_names[0][6] = "my_struct.double";
		variable_names[0][7] = "my_struct.array";
		variable_names[0][8] = "my_struct.string";
		variable_names[0][9] = "my_struct.cell";
		variable_names[0][10] = "my_struct.integer";

		variable_names[1][0] = "my_struct";
		variable_names[1][1] = "cell";
		variable_names[1][2] = "my_struct.array";
		variable_names[1][3] = "my_struct.logical";
		variable_names[1][4] = "my_struct.your_struct";
		variable_names[1][5] = "my_struct.your_struct.integer";
		variable_names[1][6] = "my_struct.your_struct.double";

		variable_names[2][0] = "my_struct";
		variable_names[2][1] = "my_struct.array";

		variable_names[3][0] = "my_struct";
		variable_names[3][1] = "md_struct";
		variable_names[3][2] = "my_struct.cell";
		variable_names[3][3] = "md_struct.double";

		variable_names[4][0] = "md_struct";
		variable_names[4][1] = "md_struct.double";
		variable_names[4][2] = "md_struct.animal";
		variable_names[4][3] = "md_array";
	}
	else
	{
		num_files = 1;
		num_vars = (int *)malloc(sizeof(int));
		num_vars[0] = 1;
	}


	for (int i=0; i < NUM_FILES; i++)
	{
		for (int j=0; j < num_vars[i]; j++)
		{
			//begin test case
			if (test_type == SINGLE_TEST)
			{
				//filename = (char *)malloc(strlen(argv[1]));
				//variable_name = (char *)malloc(strlen(argv[2]));
				strcpy(filename, argv[1]);
				strcpy(variable_name, argv[2]);
			}
			else
			{
				strcpy(filename, filenames[i]);
				strcpy(variable_name, variable_names[i][j]);
			}

			int* num_objs = (int *)malloc(sizeof(int));
			Data* objects = getDataObject(filename, variable_name, num_objs);
			Data* hi_objects = organizeObjects(objects, *num_objs);
			int index = 0;

			printf("Variable Name: %s\n", variable_name);
			while (hi_objects[index].type != UNDEF)
			{
				switch (hi_objects[index].type)
				{
					case DOUBLE:
						printDouble(&hi_objects[index]);
						break;
					case CHAR:
						printChar(&hi_objects[index]);
						break;
					case UINT16_T:
						printShort(&hi_objects[index]);
						break;
					case REF:
						printCell(&hi_objects[index]);
						break;
					case STRUCT:
						printStruct(&hi_objects[index]);
						break;
					default:
						break;
				}
				index++;
			}
			fflush(stdout);
			freeDataObjects(objects, *num_objs);
			free(hi_objects);
			printf("\n***********************************************************************\n");
		}
	}
	fclose(stdout);
}
void printDouble(Data* object)
{
	int num_elems = 1;
	int num_dims = 0;
	int i = 0;
	while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}

	printf("\n%s:\n", object->name);
	for (i = 0; i < num_elems; i++)
	{
		printf("%f ", object->double_data[i]);
		for (int j = 0; j < num_dims - 1; j++)
		{
			if ((i + 1) % object->dims[j] == 0)
			{
				printf("\n");
			}
		}
	}
	printf("\n");
}
void printShort(Data* object)
{
	int num_elems = 1;
	int num_dims = 0;
	int i = 0;
	while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}

	char* string = (char *)malloc(num_elems + 1);

	for (i = 0; i < num_elems; i++)
	{
		string[i] = object->ushort_data[i];
	}
	string[num_elems] = 0;
	printf("\n%s:\n", object->name);
	printf("%s\n\n", string);
}
void printCell(Data* object)
{
	printf("\n%s:\n", object->name);
	Data* cell_objects = object->sub_objects;

	uint16_t ushort_data;

	int num_elems;
	int num_dims_object;
	int d;

	int num_objects = 1;
	int num_dims = 0;
	d = 0;
	while(object->dims[d] > 0)
	{
		num_objects *= object->dims[d];
		num_dims++;
		d++;
	}

	for (int i = 0; i < num_objects; i++)
	{
		num_elems = 1;
		num_dims_object = 0;
		d = 0;
		while(cell_objects[i].dims[d] > 0)
		{
			num_elems *= cell_objects[i].dims[d];
			num_dims_object++;
			d++;
		}
		for (int j = 0; j < num_elems; j++)
		{
			switch (cell_objects[i].type)
			{
				case UINT16_T:
					ushort_data = cell_objects[i].ushort_data[j];

					if (j == num_elems - 1)
					{
						printf("%c ", (char)ushort_data);
					}
					else
					{
						printf("%c", (char)ushort_data);
					}
					break;
				case DOUBLE:
					printf("%f ", cell_objects[i].double_data[j]);
					break;
				default:
					printf("Cell object %d has other type: %d\n", i, cell_objects[i].type);
					break;
			}
			//fflush(stdout);
		}
		for (int j = 0; j < num_dims - 1; j++)
		{
			if ((i + 1) % object->dims[j] == 0)
			{
				printf("\n");
			}
		}
			
	}
	printf("\n");
}
void printStruct(Data* object)
{
	if (object->sub_objects == NULL)
	{
		printf("Object %s does not have a field of the requested name.\n", object->name);
		return;
	}

	printf("\n%s fields: \n", object->name);
	int index = 0;
	while (object->sub_objects[index].type != UNDEF)
	{
		printf("%s\n", object->sub_objects[index].name);
		index++;
	}

	printf("\n");
}
void printChar(Data* object)
{
	printf("%s:\n", object->name);

	int num_elems = 1;
	int num_dims = 0;
	int i = 0;
	while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}

	for (i = 0; i < num_elems; i++)
	{
		printf("%d ", object->char_data[i]);
		for (int j = 0; j < num_dims - 1; j++)
		{
			if ((i + 1) % object->dims[j] == 0)
			{
				printf("\n");
			}
		}
	}
	printf("\n");

}