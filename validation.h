#include "general.h"
#ifndef _VALIDATE_H
#define _VALIDATE_H


int validateRRJ(unsigned char data,int* stateRRJ);
int validateUA(unsigned char data,int* stateUa);
int validateRcv(unsigned char data,int* stateRcv);
int validateSET(unsigned char set,int* stateSet);
int validateDISC(unsigned char disc,int* stateDisc);

#endif
