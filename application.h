#include "validation.h"

#ifndef _APP_H
#define _APP_H




int createDtPckg(unsigned char* data,unsigned long dataSz,unsigned char** pack,int n);
int dePkgDt(unsigned char* pckg,unsigned long pckgSz,unsigned char** data,int* NC);
int dePkgCtrl(unsigned char* pckg,unsigned long pckgSz,int* start,unsigned long* tamanho,unsigned char** name);
int createCtrlPckg(unsigned char** start,unsigned char** end,int tamanho,unsigned char* name,unsigned long nameSz);
int aplRead(int fd);
int aplWrite(int fd,char* fileName);

#endif
