#include"utility.h"
#include<stdio.h>


long int glslview_loadfile(const char* cfilename, void** bufferptr){

	FILE* f;			/**/
	void* buffer;		/**/
	long length;		/**/
	long int pos;		/**/
	*bufferptr = NULL;	/**/

	if(!cfilename){
		return -1;
	}

	f = fopen(cfilename, "rb");
	if(!f){
		return -1;
	}

	/*	Get file length in bytes.	*/
    pos = ftell(f);
    fseek(f, 0,SEEK_END);
    length = ftell(f);
    fseek(f, pos, SEEK_SET);

    /*	*/
	buffer = (char*)malloc(length);
	length = fread(buffer, 1, length, f);
	fclose(f);

	*bufferptr = buffer;
	return length;
}

long int glslview_loadString(const char* cfilename, void** bufferptr){
	long int l;

	l = glslview_loadfile(cfilename, bufferptr);
	if(l > 0){
		*bufferptr = realloc(*bufferptr, l + 1);
		((char*)*bufferptr)[l] = '\0';
	}

	return l;
}
