#include "mex.h"
#include "mapping.h"

mxArray* getDouble(Data* object);
mxArray* getShort(Data* object);
char** convertToMexStringArray(uint16_t* ushort_data, int num_elems);

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

	filename = malloc((filename_len+1)*sizeof(char));
	variable_name = malloc((variable_name_len+1)*sizeof(char));

	success = mxGetString(prhs[0], filename, filename_len+1);
	if (success != 0)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:success", "Filename is invalid.");
	}

	mexPrintf("Filename: %s\n", filename);

	success = mxGetString(prhs[1], variable_name, variable_name_len+1);
	if (success != 0)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:success", "Variable name is invalid.");
	}

	mexPrintf("Variable name: %s\n", variable_name);

	int* num_objs = malloc(sizeof(int));
	int* num_super = malloc(sizeof(int));
	Data* objects = getDataObject(filename, variable_name, num_objs);
	mexPrintf("Finished fetching data objects.\n");
	Data* hi_objects = organizeObjects(objects, *num_objs, num_super);
	mexPrintf("Finished organizing data objects.\n");

	for (int i = 0; i < *num_super; i++)
	{
		mexPrintf("Type: %d\n", hi_objects[i].type);
		switch (hi_objects[i].type)
		{
			case DOUBLE_T:
				plhs[i] = getDouble(&hi_objects[i]);
				mexPrintf("Successfully fetched double.\n");
				break;
			case CHAR_T:
				//getChar(&hi_objects[index]);
				break;
			case UINTEGER16_T:
				plhs[i] = getShort(&hi_objects[i]);
				mexPrintf("Successfully fetched short.\n");
				break;
			case REF_T:
				//getCell(&hi_objects[i]);
				break;
			case STRUCT_T:
				//getStruct(&hi_objects[i]);
				break;
			default:
				break;
		}
	}

	freeDataObjects(objects, *num_objs);
	free(num_objs);
	free(hi_objects);
	free(filename);
	free(variable_name);
	free(num_super);
	mexPrintf("Complete.\n");
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

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	for (i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
	}

	//Copy data into new array that won't be freed
	mxDouble* double_data2 = mxMalloc(num_elems*sizeof(mxDouble));
	memmove(double_data2, object->double_data, num_elems*sizeof(double));

	//Create mxArray and populate it
	mxArray* array = mxCreateNumericArray(num_dims, dims, mxDOUBLE_CLASS, mxREAL);
	int success = mxSetDoubles(array, double_data2);
	if (success != 1)
	{
		mexPrintf("mxSetDoubles returned %d\n", success);
	}
	mxFree(dims);
	return array;
}
mxArray* getShort(Data* object)
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

	const char** string = convertToMexStringArray(object->ushort_data, num_elems);

	mxArray* array = mxCreateCharMatrixFromStrings(1, string);

	free((char *)string[0]);
	free((char **)string);
	return array;
}
char** convertToMexStringArray(uint16_t* ushort_data, int num_elems)
{
	char** string = (char **)malloc(sizeof(char*));
	string[0] = (char *)malloc(num_elems + 1);

	for (int i = 0; i < num_elems; i++)
	{
		string[0][i] = ushort_data[i];
	}
	string[0][num_elems] = 0;
	return string;
}
/*void getCell(Data* object)
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
				case UINTEGER16_T:
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
				case DOUBLE_T:
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

}*/