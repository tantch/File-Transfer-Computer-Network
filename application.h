#include "validation.h"

#ifndef _APP_H
#define _APP_H

struct fInfo{
	FILE * f;
	int size;
} FINFO;
struct aInfo{
	int maxSize;
	char* filename;
} AINFO;

int open_file(const char* fpath, char* mode);
int getFileSize();

int llopen(int fd,int mode);
int llread(int fd, char * buffer);
int llwrite(int fd, unsigned char* data,int tm);
int llclose(int fd);



#endif
