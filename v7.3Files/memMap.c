#include "mex.h"
#include "mapping.h"

mxArray* getDouble(Data* object);
mxArray* getShort(Data* object);
mxArray* getCell(Data* object);

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
				plhs[i] = getCell(&hi_objects[i]);
				mexPrintf("Successfully fetched cell.\n");
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
	mexPrintf("Fetching double\n");
	int num_elems = 1;
	int num_dims = 0;
	int i = 0;
	while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}
	mexPrintf("Num elements = %d\n", num_elems);

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	for (i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
		mexPrintf("dims[%d] = %d\n", i, dims[i]);
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
	mexPrintf("Returning double\n");
	return array;
}
mxArray* getShort(Data* object)
{
	mexPrintf("Fetching short\n");
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

	mxArray* array = mxCreateCharArray(num_dims, dims);

	mxChar* dataPtr = mxGetData(array);
	mexPrintf("Size of mxChar = %d\n", sizeof(mxChar));
	memmove(dataPtr, object->ushort_data, num_elems*sizeof(mxChar));
	mexPrintf("Returning short\n");
	return array;
}
mxArray* getCell(Data* object)
{
	mexPrintf("Fetching cell\n");
	Data* cell_objects = object->sub_objects;

	uint16_t ushort_data;

	int num_elems;
	int num_dims_item;
	int d;

	int num_objects = 1;
	int num_dims = 0;

	mwSize* dims;

	mxArray* cell_array;
	mxArray* cell_item;

	//Get cell array dims
	d = 0;
	while(object->dims[d] > 0)
	{
		num_objects *= object->dims[d];
		num_dims++;
		d++;
	}

	//Reverse order of dims
	dims = mxMalloc(num_dims*sizeof(mwSize));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
	}

	cell_array = mxCreateCellArray(num_dims, dims);

	for (int i = 0; i < num_objects; i++)
	{
		num_elems = 1;
		num_dims_item = 0;
		d = 0;
		while(cell_objects[i].dims[d] > 0)
		{
			num_elems *= cell_objects[i].dims[d];
			num_dims_item++;
			d++;
		}
		switch (cell_objects[i].type)
		{
			mexPrintf("Cell item type %d = %d\n", i, cell_objects[i].type);
			case UINTEGER16_T:
				cell_item = getShort(&cell_objects[i]);
				mxSetCell(cell_array, i, cell_item);
				break;
			case DOUBLE_T:
				cell_item = getDouble(&cell_objects[i]);
				mxSetCell(cell_array, i, cell_item);
				break;
			default:
				mexPrintf("Cell item %d has other type: %d\n", i, cell_objects[i].type);
				break;
		}
			
	}
	//mxFree(dims);
	mexPrintf("returning cell\n");
	return cell_array;
}
/*
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