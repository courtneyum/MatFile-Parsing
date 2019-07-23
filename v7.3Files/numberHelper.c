#include "mapping.h"


void reverseBytes(char* data_pointer, size_t num_elems)
{
	char* start,* end;
	
	for (start = data_pointer, end = start + num_elems - 1; start < end; ++start, --end )
	{
		char swap = *start;
		*start = *end;
		*start = *end;
		*end = swap;
	}
	
}
uint64_t getBytesAsNumber(char* chunk_start, int num_bytes)
{
	uint64_t ret = 0;
	int n = 0;
	uint8_t byte = 0;
	uint64_t temp = 0;
	while (n < num_bytes)
	{
		byte = *(chunk_start + n);
		temp = byte;
		temp = temp << (8*n);
		ret += temp;
		n++;
	}
	return ret;
}
double convertHexToFloatingPoint(uint64_t hex)
{
	double ret;
	int sign = 0;
	if (((uint64_t)hex & (uint64_t)pow(2, 63)) > 0)
	{
		sign = 1;
	}
	uint64_t exponent = (uint64_t)hex >> 52;
	exponent = exponent & 2047;

	uint64_t temp = (uint64_t)(pow(2, 52) - 1);
	uint64_t fraction = (uint64_t)hex & temp;

	long exp = exponent - 1023; //store in long to handle possible negative exponent
	ret = pow(-1, sign)*pow(2, exp);
	double sum = 1;

	uint64_t b_i;

	for (int i = 1; i <=52; i++)
	{
		temp = (uint64_t)pow(2, 52 - i);
		b_i = fraction & temp;
		b_i = b_i >> (52-i);
		sum += b_i*pow(2, -i);
	}

	return ret*sum;
}
int roundUp(int numToRound)
{
	int remainder = numToRound % 8;
    if (remainder == 0)
        return numToRound;

    return numToRound + 8 - remainder;
}
//indices is assumed to have the same amount of allocated memory as dims
//indices is an out parameter
void indToSub(int index, uint32_t* dims, uint64_t* indices)
{
	int num_dims = 0;
	int num_elems = 1;
	int i = 0;
	while (dims[i] > 0)
	{
		num_elems *= dims[i++];
		num_dims++;
	}

	indices[0] = (index + 1)% dims[0];

	int divide = dims[0];
	int mult = 1;
	int sub = indices[0]*mult;

	for (i = 1; i < num_dims - 1; i++)
	{
		indices[i] = ((index + 1 - sub)/divide)%dims[i];
		divide *= dims[i];
		mult*= dims[i-1];
		sub += indices[i]*mult;
	}

	divide *= dims[num_dims - 1];
	mult *= dims[num_dims - 2];
	sub += indices[num_dims - 2];
	indices[num_dims - 1] = (index + 1 - sub)/divide;
}
//here indices is an in parameter
int subToInd(uint32_t* dims, uint64_t* indices)
{
	int num_dims = 0;
	int num_elems = 1;
	int i = 0;
	while (dims[i] > 0)
	{
		num_elems *= dims[i++];
		num_dims++;
	}
	int index = dims[num_dims - 1];
	int plus = 0;
	int mult = 1;
	i = num_dims - 2;
	while (i > 0)
	{
		mult*=dims[i+1];
		plus = indices[i]*mult;
		index+= plus;
		i--;
	}
	return index;
}