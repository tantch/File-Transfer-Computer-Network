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
#define RECEIVER 0
#define WRITER 1
#define BIT(N) (0x01<<N)

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
void createSET(unsigned char* set,int mode){
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
//validar por maquina de estado

void createRR(unsigned char* rr,int Nr,int mode){

  rr[0]=F;
  if(mode == RECEIVER){
    rr[1]=A1;
  }else{
    rr[1]=A0;
  }
  unsigned char tmp = CRR0;
  if(Nr==1){
    tmp = CRR1;
  }
  rr[2]=tmp;
  rr[3]=rr[1]^rr[2];
  rr[4]=F;

}

/*
*@param rej array of bytes to be set into a REJ trama
*@param Nr defines Nr
*/
void createREJ(unsigned char* rej,int Nr,int mode){

  rej[0]=F;
  if(mode == RECEIVER){
    rej[1]=A1;
  }else{
    rej[1]=A0;
  }
  unsigned char tmp = CREJ0;
  if(Nr==1){
    tmp = CREJ1;
  }
  rej[2]=tmp;
  rej[3]=rej[1]^rej[2];
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
    }else if(data == CRR0){
      *stateRRJ=3;
    }else if(data == CRR1){
      *stateRRJ=6;
    }else{
      *stateRRJ=0;
    }
    return 0;

    case 3:
    if(data == F){
      *stateRRJ=1;
    }else if(data == (A1 ^ CRR0)){
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
    }else if(data == (A1 ^ CRR1)){
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
    if(data == (A1^C0)){
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
    if(data == (A1 ^ C1)){
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

int validateSET(unsigned char set,int* stateSet){

  switch(*stateSet){
    case 0:
    if(set == F){
      *stateSet=1;
    }
    return 0;
    case 1:
    if(set == F){
      *stateSet=1;
    }else if(set == A1){
      *stateSet=2;
    }else{
      *stateSet=0;
    }
    return 0;
    case 2:
    if(set == F){
      *stateSet=1;
    }else if(set == CSET){
      *stateSet=3;
    }else{
      *stateSet=0;
    }
    return 0;
    case 3:
    if(set == F){
      *stateSet=1;
    }else if(set == (A1 ^ CSET)){
      *stateSet=4;
    }else{
      *stateSet=0;
    }
    return 0;
    case 4:
    if(set == F){
      *stateSet=5;
      return 1;
    }

    else{
      *stateSet=0;
    }
    return 0;

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

int destuffing(char sent,char* data,char* bcc,int* escape,int* destufcount){
  if(*escape==0 && sent != 0x7d){
    data[(*destufcount)]=sent;
    *bcc=*bcc ^ data[*destufcount];
    (*destufcount)++;
    return 0;
  }
  else if(*escape==0 && sent == 0x7d){
    *escape=1;
    return 0;
  }
  else if(*escape==1 && sent == 0x5e){
    data[*destufcount]=0x7e;
    *bcc=*bcc ^ data[*destufcount];
    (*destufcount)++;
    *escape=0;
    return 0;
  }

  else if(*escape==1 && sent == 0x5d){
    data[*destufcount]=0x7d;
    *bcc=*bcc ^ data[*destufcount];
    (*destufcount)++;
    *escape=0;
    return 0;
  }
  else{
    return -1;
  }
}

int stuffing(unsigned char* data, unsigned char* stuffed, int n){
  int i,r;
  r=0;
  for(i=0;i<n;i++){
    if(data[i]==F){
      stuffed[r]=ESC_BYTE;
      stuffed[++r]=0x5E;
      r++;

    }
    else if(data[i]==ESC_BYTE){
      stuffed[r]=ESC_BYTE;
      stuffed[++r]=0x5D;
      r++;

    }
    else{
      stuffed[r]=data[i];
      r++;}

  }
  return r;
}

void BCC2(unsigned char* data, unsigned char* final, int n){
  int i;
  for (i=0;i<n-1;i++){
    *final=data[i]^data[i+1];
  }
}

void completeData(unsigned char* data, unsigned char* final,int c, int n){
  int i;
  unsigned char bcc2;
  BCC2(data,&bcc2, n);

  for (i=0;i<n + 4;i++){
    final[i+4]=data[i];
  }

  final[0]=F;
  final[1]=A1;

  if (c==0){
    final[2]=C0;
  }

  if (c==1){
    final[2]=C1;
  }

  final[3]=final[1]^final[2];
  final[n+4]=bcc2;
  final[n+5]=F;
}

void llopen(int fd,int mode){

  char buf[255];
  unsigned char recv;
  int i;
  int res;
  int rcv=0;
  int state=0;
  int ret=0;
  config(30,0,fd);

  if(mode == RECEIVER){
    unsigned char ua[5];
    createUA(ua,mode);

    do{
      res= read(fd,buf,1);
      if(res!=0){
        recv=buf[0];
        ret=validateSET(recv,&state);
      }
      else if(res==0){
        ret=0;
        //state=0;
      }

    }while(ret==0);

    res=write(fd,ua,5);

  }
  else{
    while(rcv==0){
      unsigned char set[5];
      createSET(set,mode);
      res=write(fd,set,5);


      do{
        res= read(fd,buf,1);
        if(res!=0){
          recv=buf[0];
          ret=validateUA(recv,&state);
        }
        else if(res==0){
          ret=-1;
        }
      }while(ret==0);

      if(ret==1){
        rcv=1;
      }
    }


  }
}

int llread(int fd, char * buffer){
  char rec;
  char buf[255];
  int r;
  int stateData;

  printf("a começar a ler...\n");
  int i=0;
  do{

  r=read(fd,buf,1);
  if(r!=0)
    rec=buf[0];
  else if(r==0){
    printf("nothing received!\n");
    rec=0x11;}
  if (stateData==7){
    buffer[i]=rec;
    i++;}


  }while(validateRcv(rec, stateData)==0);
  return (i--);
}

//falta mandar mensagens ao emissor

void printChar(unsigned char* cena,int tam){
  int i;
  for(i=0; i<tam;i++){
    printf("char[%i]:0x%x\n",i,cena[i]);
  }
}

int main(int argc,unsigned char** argv)
{
  int fd,c;
  struct termios oldtio;
  int user = RECEIVER;

  /*fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1){
      perror("tcgetattr");
      exit(-1);
    }

    //llopen(fd,user);*/

    /*
    unsigned char test[5];
    createRR(test,0,user);
    printChar(test,5);
    createRR(test,1,user);
    printChar(test,5);
    createREJ(test,0,user);
    printChar(test,5);
    createREJ(test,1,user);
    printChar(test,5);

    */

    unsigned char data[5];
    data[0]=0x11;
    data[1]=F;
    data[2]=0x7d;
    data[3]=0x69;
    data[4]=0x01;

    printChar(data,5);
    printf("--------------\n");
    unsigned char stuffed_tmp[10];
    int red=stuffing(data,stuffed_tmp,5);
    printChar(stuffed_tmp,red);

printf("--------------\n");

    unsigned char destuffed_tmp[5];
    int esc=0;
    int i=0;
    int j=0;
    unsigned char bcctmp=0x00;
    for(j=0;j<red;j++){
      destuffing(stuffed_tmp[j],destuffed_tmp,&bcctmp, &esc,&i);
    }
    printf("i:%i\n",i);
    printChar(destuffed_tmp,i);
printf("--------------\n");
    unsigned char final[red+6];
    completeData(stuffed_tmp,final,0,red);
    printChar(final,red+6);


    sleep(3);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    exit(0);
  }
