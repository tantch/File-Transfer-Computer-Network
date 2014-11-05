#include "general.h"
#ifndef _LINK_H
#define _LINK_H

int llopen(int fd,int mode);
int llread(int fd, char * buffer);
int llwrite(int fd, unsigned char* data,int tm);
int llclose(int fd);
void BCC2(unsigned char* data, unsigned char* final, int n);
int completeData(unsigned char* data, unsigned char* final,int c, int n,unsigned char bcc2);
int stuffing(unsigned char* data, unsigned char* stuffed, int n);
int destuffing(char sent,char* data,char* bcc,int* escape,int* destufcount);

#endif
