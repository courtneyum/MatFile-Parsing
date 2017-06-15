#include <stdio.h>
#include "zlib.h"
#include <string.h>
#include <stdlib.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

#define HEADER_SIZE 128

int numFromStr(char* str);

int main(int argc, char* argv[])
{
	char filename[] = "my_struct.mat";
	int bytesToRead, readErr, comprErr;
	char *buffer, *uncompr;
	uLongf uncomprLen;

	FILE *fp, *uncomprfp;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		perror("Could not open compressed data file.");
		exit(1);
	}

	//skip header
	fseek(fp, HEADER_SIZE, SEEK_SET);

	//get number of bytes in data element
	fseek(fp, 4, SEEK_CUR);
	char dataSize[4];
	if (fread(dataSize, 1, 4, fp) != 4)
	{
		fputs("Read failed on line 41", stderr);
	}
	bytesToRead = numFromStr(dataSize);

	//read compressed data element into buffer
	buffer = (char *)malloc(bytesToRead);
	if (readErr = fread(buffer, 1, bytesToRead, fp) != bytesToRead)
	{
		fputs("Read failed on line 50.", stderr);
	}

	//uncompress compressed data element
	uncomprLen = 2*bytesToRead;
	uncompr = (char *)malloc(uncomprLen);

	comprErr = uncompress(uncompr, &uncomprLen, buffer, bytesToRead);
    CHECK_ERR(comprErr, "uncompress");

    //write uncompressed data element to file
    uncomprfp = fopen("UncompressedDataElement.txt", "w");
    if (fwrite(uncompr, 1, uncomprLen, uncomprfp) != uncomprLen)
    {
    	fputs("Write failed.", stderr);
    }

    fclose(fp);
    fclose(uncomprfp);
}
int numFromStr(char* str)
{
	return str[0] + (str[1] << 8) + (str[2] << 16) + (str[3] << 24);
}