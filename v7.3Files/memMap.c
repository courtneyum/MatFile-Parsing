#include "mapping.h"
#include "mex.h"

mxArray* getDouble(Data* object);
mxArray* getShort(Data* object);
mxArray* getCell(Data* object);
mxArray* getStruct(Data* object);
mxArray* getMDStruct(Data* sub_objects, const char** fieldnames, int num_fields);
mxArray* getScalarStruct(Data* sub_objects, const char** fieldnames, int num_fields);
mxArray* getChar(Data* object);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char* filename;
	char* variable_name;
	int success;

	if (nrhs < 2)
	{
		mexErrMsgIdAndTxt("MyToolbox:memMap:nrhs", "Two inputs required: filename and variable_name.");
	}

	mwSize filename_len = mxGetN(prhs[0]);
	mwSize variable_name_len = mxGetN(prhs[1]);

	filename = mxMalloc((filename_len+1)*sizeof(char));
	variable_name = mxMalloc((variable_name_len+1)*sizeof(char));

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

	int num_objs;
	int num_super;
	Data* objects = getDataObject(filename, variable_name, &num_objs);
	mexPrintf("Finished fetching data objects.\n");
	Data* hi_objects = organizeObjects(objects, num_objs, &num_super);
	mexPrintf("Finished organizing data objects.\n");

	for (int i = 0; i < num_super; i++)
	{
		mexPrintf("Type: %d\n", hi_objects[i].type);
		switch (hi_objects[i].type)
		{
			case DOUBLE_T:
				plhs[i] = getDouble(&hi_objects[i]);
				mexPrintf("Successfully fetched double.\n");
				break;
			case CHAR_T:
				plhs[i] = getChar(&hi_objects[i]);
				mexPrintf("Successfully fetched logical.\n");
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
				plhs[i] = getStruct(&hi_objects[i]);
				mexPrintf("Successfully fetched struct.\n");
				break;
			default:
				break;
		}
	}

	freeDataObjects(objects, num_objs);
	freeDataObjects(hi_objects, num_super);
	mxFree(filename);
	mxFree(variable_name);
	mexPrintf("Complete.\n");
}
mxArray* getDouble(Data* object)
{
	mexPrintf("Fetching double\n");
	int num_elems = 1;
	int num_dims = 0;
	int i = 0;
	/*while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}*/
	num_elems = object->num_elems;
	num_dims = object->num_dims;
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
	/*int i = 0;
	while(object->dims[i] > 0)
	{
		num_elems *= object->dims[i];
		num_dims++;
		i++;
	}*/
	num_elems = object->num_elems;
	num_dims = object->num_dims;

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
	}

	mxArray* array = mxCreateCharArray(num_dims, dims);

	mxChar* dataPtr = mxGetData(array);
	memmove(dataPtr, object->ushort_data, num_elems*sizeof(mxChar));
	mxFree(dims);
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
	/*d = 0;
	while(object->dims[d] > 0)
	{
		num_objects *= object->dims[d];
		num_dims++;
		d++;
	}*/
	num_objects = object->num_elems;
	num_dims = object->num_dims;

	//Reverse order of dims
	dims = mxMalloc(num_dims*sizeof(mwSize));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
	}

	cell_array = mxCreateCellArray(num_dims, dims);

	for (int i = 0; i < num_objects; i++)
	{
		/*num_elems = 1;
		num_dims_item = 0;
		d = 0;
		while(cell_objects[i].dims[d] > 0)
		{
			num_elems *= cell_objects[i].dims[d];
			num_dims_item++;
			d++;
		}*/
		switch (cell_objects[i].type)
		{
			mexPrintf("Cell item type %d = %d\n", i, cell_objects[i].type);
			case UINTEGER16_T:
				cell_item = getShort(&cell_objects[i]);
				break;
			case DOUBLE_T:
				cell_item = getDouble(&cell_objects[i]);
				break;
			case CHAR_T:
				//logical
				cell_item = getChar(&cell_objects[i]);
				break;
			default:
				mexPrintf("Cell item %d has other type: %d\n", i, cell_objects[i].type);
				break;
		}
		mxSetCell(cell_array, i, cell_item);
			
	}
	mxFree(dims);
	mexPrintf("returning cell\n");
	return cell_array;
}

mxArray* getStruct(Data* object)
{
	if (object->sub_objects == NULL)
	{
		mexPrintf("Object %s does not have a field of the requested name.\n", object->name);
		return NULL;
	}
	
	mxArray* array;

	//Get field names
	mexPrintf("Fetching fieldnames\n");
	Data* sub_objects = object->sub_objects;
	int num_fields = object->num_sub_objects;
	char** fieldnames = mxMalloc(num_fields*sizeof(char*));
	for (int i = 0; i < num_fields; i++)
	{
		fieldnames[i] = mxMalloc(strlen(sub_objects[i].name)*sizeof(char));
		strcpy(fieldnames[i], sub_objects[i].name);
		mexPrintf("Fieldname[%d] = %s\n", i, fieldnames[i]);
	}

	//Is it scalar or multi dimensional?
	mexPrintf("Getting struct array dims\n");
	char* null_str = malloc(CLASS_LENGTH*sizeof(char));
	memset(null_str, '\0', CLASS_LENGTH*sizeof(char));
	if (sub_objects[0].type == REF_T && strncmp(sub_objects[0].matlab_class, null_str, CLASS_LENGTH) == 0)
	{
		mexPrintf("Fetching md struct\n");
		array = getMDStruct(sub_objects, (const char**)fieldnames, num_fields);
		mexPrintf("Fetched md struct\n");
	}
	else
	{
		mexPrintf("Fetching scalar struct\n");
		array = getScalarStruct(sub_objects, (const char**)fieldnames, num_fields);
		mexPrintf("Fetched scalar struct\n");
	}

	//Free fieldnames
	mexPrintf("Freeing fieldnames\n");
	for (int i = 0; i < num_fields; i++)
	{
		mxFree(fieldnames[i]);
	}
	mxFree(fieldnames);

	free(null_str);

	mexPrintf("Returning struct\n");
	return array;
}

mxArray* getMDStruct(Data* sub_objects, const char** fieldnames, int num_fields)
{
	uint32_t* object_dims = sub_objects[0].dims;
	uint32_t num_dims = sub_objects[0].num_dims;
	uint32_t num_elems = sub_objects[0].num_elems;
	Data* current_obj;
	mxArray* field;

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object_dims[num_dims - 1 - i];
		mexPrintf("dims[%d] = %d\n", i, dims[i]);
	}

	//Create struct array
	mxArray* array = mxCreateStructArray(num_dims, dims, num_fields, fieldnames);

	//Populate struct array
	mexPrintf("Populating struct array\n");
	mexPrintf("Num fields = %d\n", num_fields);
	for (int i = 0; i < num_fields; i++)
	{
		mexPrintf("Num sub objects = %d\n", sub_objects[i].num_sub_objects);
		for (int j = 0; j < sub_objects[i].num_sub_objects; j++)
		{
			current_obj = &(sub_objects[i].sub_objects[j]);
			switch (current_obj->type)
			{
				case UINTEGER16_T:
					field = getShort(current_obj);
					break;
				case DOUBLE_T:
					field = getDouble(current_obj);
					break;
				case REF_T:
					field = getCell(current_obj);
					break;
				case CHAR_T:
					//logical
					field = getChar(current_obj);
					break;
				case STRUCT_T:
					field = getStruct(current_obj);
					break;
				default:
					mexPrintf("Struct field %d has other type: %d\n", i, sub_objects[i].type);
					break;
			}
			mexPrintf("Setting field %d at index %d\n", i, j);
			mxSetFieldByNumber(array, j, i, field);
		}
	}
	mxFree(dims);
	return array;
}

mxArray* getScalarStruct(Data* sub_objects, const char** fieldnames, int num_fields)
{
	uint32_t num_dims = 2;
	uint32_t num_elems = 1;
	Data* current_obj;
	mxArray* field;

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	dims[0] = 1;
	dims[1] = 1;

	//Create struct array
	mxArray* array = mxCreateStructArray(num_dims, dims, num_fields, fieldnames);

	mexPrintf("Num fields = %d\n", num_fields);
	for (int i = 0; i < num_fields; i++)
	{
		current_obj = &sub_objects[i];
		switch (current_obj->type)
		{
			case UINTEGER16_T:
				field = getShort(current_obj);
				break;
			case DOUBLE_T:
				field = getDouble(current_obj);
				break;
			case REF_T:
				field = getCell(current_obj);
				break;
			case CHAR_T:
				//logical
				field = getChar(current_obj);
				break;
			case STRUCT_T:
				field = getStruct(current_obj);
				break;
			default:
				mexPrintf("Struct field %d has other type: %d\n", i, sub_objects[i].type);
				break;
		}
		mexPrintf("Setting field %d at index %d\n", i, 1);
		mxSetFieldByNumber(array, 0, i, field);
	}
	mxFree(dims);
	return array;
}
mxArray* getChar(Data* object)
{
	int num_elems = object->num_elems;
	int num_dims = object->num_dims;

	//Reverse order of dims
	mwSize* dims = mxMalloc(num_dims*sizeof(mwSize));
	for (int i = 0; i < num_dims; i++)
	{
		dims[i] = object->dims[num_dims - 1 - i];
		mexPrintf("dims[%d] = %d\n", i, dims[i]);
	}

	mxArray* array = mxCreateLogicalArray(num_dims, dims);
	mxLogical* dataPtr = mxGetData(array);
	memmove(dataPtr, object->char_data, num_elems*sizeof(mxLogical));

	mxFree(dims);
	return array;

}