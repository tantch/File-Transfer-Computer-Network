#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE 1
#define F 0x7E
#define A0 0x01
#define A1 0x03
#define CSET 0x03
#define CUA 0x07
#define C0 0x00
#define C1 0x40
#define RECEIVER 0
#define WRITER 1


#define BIT(n)    (0x01<<n)



//criar as tramas

/*
*@param ua array of bytes to be set into a ua trama
*@param mode defines receiver or writter mode
*/
void createUA(unsigned char* ua,int mode){
  ua[0]=F;
  if(mode==RECEIVER){
    ua[1]=A1;
  }else{
    ua[1]=A0;
  }
  ua[2]=CUA;
  ua[3]=ua[1]^ua[2];
  ua[4]=F;
}
/*
*@param set array of bytes to be set into a set trama
*@param mode defines receiver or writter mode
*/
void createSet(unsigned char* set,int mode){
  set[0]=F;
  if(mode == WRITER){
    set[1]=A1;
  }else{
    set[1]=A0;
  }
  set[2]=CSET;
  set[3]=set[1]^set[2];
  set[4]=F;
}

/*
*@param rr array of bytes to be set into a RR trama
*@param Nr defines Nr
*/
void createRR(unsigned char* rr,int Nr){

  rr[0]=F;
  rr[1]=A;
  char tmp = 0x05;
  if(Nr==1){
    tmp = tmp | BIT(8);
  }
  rr[2]=tmp;
  rr[3]=rr[0]^r[1]^r[2];
  rr[4]=F;

}

/*
*@param rej array of bytes to be set into a REJ trama
*@param Nr defines Nr
*/
void createREJ(unsigned char* rej,int Nr){

  rej[0]=F;
  rej[1]=A;
  char tmp = 0x01;
  if(Nr==1){
    tmp = tmp | BIT(8);
  }
  rej[2]=tmp;
  rej[3]=rr[0]^r[1]^r[2];
  rej[4]=F;

}

//validar por maquina de estado

/*
*@param data byte a comparar para alterar a M.E.
*@param stateRRJ estado da maquina
*@ret int -1->erro(mandar tudo abaixo)
*     0-> continuar
*     1-> recebido trama de confirmaçao RR com N(r)=0
*     2-> recebido trama de confirmação RR com N(r)=1
*     3-> recebido trama de rejeição REJ com N(r)=0
*     4-> recebido trama de rejeição REJ com N(r)=0
*/
int validateRRJ(unsigned char data,int* stateRRJ){
  switch(*stateRRJ){
    case 0:
    if(data==F){
      *stateRRJ=1;
    }
    return 0;

    case 1:
    if(data == F){
      *stateRRJ=1;
    }
    else if(data == A1){
      *stateRRJ=2;
    }
    else{
      *stateRRJ=0;
    }
    return 0;

    case 2:
    if(data == F){
      *stateRRJ=1;
    }else if(data == CRR1){
      *stateRRJ=3;
    }else if(data == CRR2){
      *stateRRJ=6;
    }else{
      *stateRRJ=0;
    }
    return 0;

    case 3:
    if(data == F){
      *stateRRJ=1;
    }else if(data == (A1 ^ CRR1)){
      *stateRRJ=4;
    }else{
      *stateRRJ=0;
    }
    return 0;

    case 4:
    if(data == F){
      *stateRRJ=5;
      return 1;
    }else{
      *stateRRJ=0;
      return 0;
    }
    case 5:
    return -1;
    case 6:
    if(data == F){
      *stateRRJ=1;
    }else if(data == (A1 ^ CRR2)){
      *stateRRJ=7;
    }else{
      *stateRRJ=0;
    }
    return 0;

    case 7:
    if(data == F){
      *stateRRJ=8;
      return 2;
    }else{
      *stateRRJ=0;
      return 0;
    }
    case 8:
    return -1;
  }
}

/*
*@param data byte to alter the state machine
*@param stateUa state of the machine to be altered
*@ret 0->continue
*     1->machine validated an Ua trama
*     -1->error end aplication
*/
int validateUA(unsigned char data,int* stateUa){
  switch(*stateUa){
    case 0:
    if(data==F){
      *stateUa=1;
    }
    return 0;
    case 1:
    if(data == F){
      *stateUa=1;
    }else if(data == A1){
      *stateUa=2;
    }else{
      *stateUa=0;
    }
    return 0;

    case 2:
    if(data == F){
      *stateUa=1;
    }else if(data == CUA){
      *stateUa=3;
    }else{
      *stateUa=0;
    }
    return 0;

    case 3:
    if(data == F){
      *stateUa=1;
    }else if(data == (A1 ^ CUA)){
      *stateUa=4;
    }	else{
      *stateUa=0;
    }
    return 0;

    case 4:
    if(data == F){
      *stateUa=5;
      return 1;
    }else{
      *stateUa=0;
      return 0;
    }
    case 5:
    return -1;
  }
}
/*
*@param data byte to alter the state machine
*@param stateRcv state of the machine to be altered
*@ret 0->continue
*     1->machine validated a SET trama
*     2->machine validated a Data trama with N(r)=0
*     3->machine valdiated a DAta trama with N(r)=1
*     -1->error end aplication
*/
int validateRcv(unsigned char data,int* stateRcv){
  switch(*stateRcv){
    case 0:
    if(data == F){
      *stateRcv=1;
    }
    return 0;

    case 1:
    if(data == F){
      *stateRcv=1;
    }else if(data == A1){
      *stateRcv=2;
    }else{
      *stateRcv=0;
    }
    return 0;

    case 2:
    if(data == F){
      *stateRcv=1;
    }else if(data == CSET){
      *stateRcv=3;
    }else if(data == C0){
      *stateRcv=6;
    }else if(data == C1){
      *stateRcv=9;
    }else{
      *stateRcv=0;
    }
    return 0;

    case 3:
    if(data == F){
      *stateRcv=1;
    }else if(data == (A1 ^ CSET)){
      *stateRcv=4;
    }else{
      *stateRcv=0;
    }
    return 0;

    case 4:
    if(data == F){
      *stateRcv=5;
      return 1;
    }else{
      *stateRcv=0;
      return 0;
    }
    case 5:
    return -1;

    case 6:
    if(data == BCC1){
      *stateRcv=7;
    }else if(data == F){
      *stateRcv=1;
    }else{
      *stateRcv=0;
    }
    return 0;

    case 7:
    if(data == F){
      *stateRcv = 8;
      return 2;
    }else{
      *stateRcv=7;
    }return 0;

    case 8:
    return -1;

    case 9:
    if(data == BCC1){
      *stateRcv=10;
    }else if(data== F){
      *stateRcv=1;
    }	else{
      *stateRcv=0;
    }
    return 0;

    case 10:
    if(data == F){
      *stateRcv=11;
      return 3;
    }
    else{
      *stateRcv=10;
      return 0;
    }
    case 11:
    return -1;
    default:
    return -1;
  }
}

/*configures the configurations of the serial port
*
*@param vtim tempo que demora a sair do read quando nao esta a ler nada
*@param vmin numero minimo de bytes qe tem que ler
*@param fd path para a porta de serie
*/
void config(int vtime,int vmin,int fd){
  struct termios newtio;

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;


  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = vtime;
  newtio.c_cc[VMIN]     = vmin;
  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
}

void destuffing(char sent,char* data,char* bcc,int* escape,int* destufcount){

	if(*escape==0 && sent != 0x7d){
		data[*destufcount]=sent;
		*bcc=*bcc ^ data[*destufcount];
		*destufcount++;
		return 0;
	}
	else if(*escape==0 && sent == 0x7d){
		*escape=1;
		return 0;


	}
	else if(*escape==1 && sent == 0x5e){
		data[*destufcount]=0x7e;
		*bcc=*bcc ^ data[*destufcount];
		*destufcount++;
		*escape=0;
		return 0;
	}

	else if(*escape==1 && sent == 0x5d){
		data[*destufcount]=0x7d;
		*bcc=*bcc ^ data[*destufcount];
		*destufcount++;
		*escape=0;
		return 0;
	}
	else{
		return -1;
	}
}

void llopen((int fd,int mode){

  char buf[255];
  unsigned char recv;
  int i;
  int res;
  int rcv=0;
  config(30,0,fd);

  if(mode == RECEIVER){
    unsigned char ua[5];
    createUA(&ua);

    do{
      res= read(fd,buf,1);
      if(res!=0){
        recv=buf[0];
      }
      else if(res==0){
        recv=0x11;
      }

    }while(validateSET(recv)==0);

    res=write(fd,ua,5);

  }
  else{
    while(rcv==0){
		res=write(fd,set,5);



		while(counter!=5){
			res= read(fd,buf,1);
			if(res!=0){
				recv[counter]=buf[0];
				counter++;
			}
			if(res==0 || buf[0]==0)
				stop=1;
		}

		if(validateUA(&recv)==1)
			rcv=1;
	}


  }
}

int main(int argc,unsigned char** argv)
{
  int fd,c;
  struct termios oldtio;
  int user = RECEIVER;

  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1){
    perror("tcgetattr");
    exit(-1);
  }

  //llopen(fd,user);



  sleep(3);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  exit(0);
}
