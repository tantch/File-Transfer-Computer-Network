#ifndef _APP_H
#define _APP_H

#include "general.h"
#include "validation.h"

int llopen(int fd,int mode);
int llread(int fd, char * buffer);
void llwrite(int fd, unsigned char* data);
int llclose(int fd);



#endif
