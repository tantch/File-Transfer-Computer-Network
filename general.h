
#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#ifndef _GENERAL_H
#define _GENERAL_H


#define BAUDRATE B38400
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE 1
#define F 0x7E
#define ESC_BYTE 0x7D
#define A0 0x01
#define A1 0x03
#define CSET 0x03
#define CUA 0x07
#define C0 0x00
#define C1 0x40
#define CRR0 0x05
#define CRR1 0x85
#define CREJ0 0x01
#define CREJ1 0x81
#define CDISC 0x0B

#define RECEIVER 0
#define WRITER 1

#define BIT(N) (0x01<<N)

#define TIMEOUT 1
#define RETRANSMIT 3


int nTimeouts;
int alarm_flag;
int MODE;
int Ns;


void printChar(unsigned char* cena,int tam);
void alarmhandler(int signo);

void createUA(unsigned char* ua,int mode);
void createSET(unsigned char* set,int mode);
void createDISC(unsigned char* set,int mode);
void createRR(unsigned char* rr,int Nr,int mode);
void createREJ(unsigned char* rej,int Nr,int mode);

void BCC2(unsigned char* data, unsigned char* final, int n);

#endif
