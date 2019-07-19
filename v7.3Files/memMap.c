#include "mex.h"
#include "mapping.h"
#include "matrix.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char* filename;
	char* variable_name;
	int success, index;

	if (nrhs < 2)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:nrhs", "Two inputs required: filename and variable_name.");
	}

	mwSize filename_len = mxGetN(prhs[0]);
	mwSize variable_name_len = mxGetN(prhs[1]);

	filename = (char *)malloc(filename_len+1);
	variable_name_len = (char *)malloc(variable_name_len+1);

	success = !mxGetString(prhs[0], filename, filename_len+1);
	if (!success)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:success", "Filename is invalid.");
	}

	success = !mxGetString(prhs[1], variable_name, variable_name_len+1);
	if (!success)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:success", "Variable name is invalid.");
	}

	int* num_objs = (int *)malloc(sizeof(int));
	Data* objects = getDataObject(filename, variable_name, num_objs);
	Data* hi_objects = organizeObjects(objects, *num_objs);

	free(objects)

	index = 0;
	while (hi_objects[index].type != UNDEF)
	{
		switch (hi_objects[index].type)
		{
			case DOUBLE:
				plhs[index] = getDouble(&hi_objects[index]);
				break;
			case CHAR:
				getChar(&hi_objects[index]);
				break;
			case UINT16_T:
				getShort(&hi_objects[index]);
				break;
			case REF:
				getCell(&hi_objects[index]);
				break;
			case STRUCT:
				getStruct(&hi_objects[index]);
				break;
			default:
				break;
		}
		index++;
	}
}
mxArray* getDouble(Data* object)
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

	mxArray* array = mxCreateNumericArray(num_dims, object->dims, mxDOUBLE_CLASS, FALSE);
	int* dim_counters = (int *)calloc(num_dims, sizeof(int));

	int success = !mxSetDoubles(array, object->double_data);
}
void getShort(Data* object)
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
void getCell(Data* object)
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
void getStruct(Data* object)
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
void getChar(Data* object)
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