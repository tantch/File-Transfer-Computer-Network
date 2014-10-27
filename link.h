#include "general.h"
#ifndef _LINK_H
#define _LINK_H


int createDtPckg(unsigned char* data,unsigned long dataSz,unsigned char** pack,int n);
int dePkgDt(unsigned char* pckg,unsigned long pckgSz,unsigned char** data);
int dePkgCtrl(unsigned char* pckg,unsigned long pckgSz,int* start,unsigned long* tamanho,unsigned char** name);
int createCtrlPckg(unsigned char** start,unsigned char** end,int tamanho,unsigned char* name,unsigned long nameSz);
off_t fsize(char *filename);



#endif
