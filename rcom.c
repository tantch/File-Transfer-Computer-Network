#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>


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

#define TIMEOUT 3
#define RETRANSMIT 3

int nTimeouts=0;
int alarm_flag=0;
int MODE;

void alarmhandler(int signo) {

  printf("alarm handler\n");
  alarm_flag=1;

}

//criar as tramas
void printChar(unsigned char* cena,int tam){
  int i;
  for(i=0; i<tam;i++){
    printf("char[%i]:0x%x\n",i,cena[i]);
  }
}

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

void createDISC(unsigned char* set,int mode){
  set[0]=F;
  if(mode == WRITER){
    set[1]=A1;
  }else{
    set[1]=A0;
  }
  set[2]=CDISC;
  set[3]=set[1]^set[2];
  set[4]=F;
}

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

int validateDISC(unsigned char disc,int* stateDisc){

  switch(*stateDisc){
    case 0:
    if(disc == F){
      *stateDisc=1;
    }
    return 0;
    case 1:
    if(disc == F){
      *stateDisc=1;
    }else if(disc == A1 || disc == A0){
      *stateDisc=2;
    }else{
      *stateDisc=0;
    }
    return 0;
    case 2:
    if(disc == F){
      *stateDisc=1;
    }else if(disc == CDISC){
      *stateDisc=3;
    }else{
      *stateDisc=0;
    }
    return 0;
    case 3:
    if(disc == F){
      *stateDisc=1;
    }else if(disc == (A1 ^ CDISC)){
      *stateDisc=4;
    }else{
      *stateDisc=0;
    }
    return 0;
    case 4:
    if(disc == F){
      *stateDisc=5;
      return 1;
    }

    else{
      *stateDisc=0;
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
      r++;
    }

  }
  return r;
}

void BCC2(unsigned char* data, unsigned char* final, int n){
  int i;
  for (i=0;i<n;i++){
    *final=data[i]^*final;
  }
}

void completeData(unsigned char* data, unsigned char* final,int c, int n,unsigned char bcc2){
  int i;


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

int llopen(int fd,int mode){

  char buf[255];
  unsigned char recv;
  int i;
  int res;
  int rcv=0;
  int state=0;
  int ret=0;
  config(3,0,fd);
  MODE=mode;
  if(mode == RECEIVER){
    unsigned char ua[5];
    createUA(ua,mode);
    alarm(TIMEOUT);
    do{

      res= read(fd,buf,1);
      if(res!=0){
        alarm(TIMEOUT);
        recv=buf[0];
        ret=validateSET(recv,&state);
      }
      else if(res==0){
        ret=0;
        if(alarm_flag==1){
          alarm_flag=0;
          nTimeouts++;
			alarm(TIMEOUT);
        }
      }
	printf("ret:%i\ntimeouts:%i\n",ret,nTimeouts);
    }while(ret==0 && nTimeouts<RETRANSMIT);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection.\n");
      return -1;
    }

    res=write(fd,ua,5);
    nTimeouts=0;
    return 0;
  }

  else{

    unsigned char set[5];
    createSET(set,mode);
    while(nTimeouts<RETRANSMIT && res<1){
      res=write(fd,set,5);
      alarm(TIMEOUT);
      do{

        res= read(fd,buf,1);

        if(res==1){
          alarm(TIMEOUT);
          recv=buf[0];
          ret=validateUA(recv,&state);
          nTimeouts=0;
        }
        else if(res==0){
          ret=0;
          if(alarm_flag==1){
            alarm_flag=0;
            ret=-1;
            nTimeouts++;
          }
        }
      }while(ret==0);
    }
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection.\n");
      return -1;
    }
    nTimeouts=0;
    return 0;

  }
}

int llread(int fd, char * buffer){
  char rec,ret;
  char buf[255];
  int r;
  int stateData=0;
  char bccData=0x00;
  int esc=0;
  int i=0;

  printf("a começar a ler...\n");

  alarm(TIMEOUT);
  do{
    r= read(fd,buf,1);
	if(r==-1){
		printf("erro:%s\n",strerror(errno));	
}
    if(r==1){
      alarm(TIMEOUT);
      rec=buf[0];
      if(rec!= 0x7e){
        ret=validateRcv(rec,&stateData);
        destuffing(rec,buffer,&bccData, &esc,&i);
      }
    }
    else if(r==0){
      ret=0;
      if(alarm_flag==1){
        alarm_flag=0;
        nTimeouts++;
		alarm(TIMEOUT);
      }
    }
  }while(ret==0 && nTimeouts<RETRANSMIT);
	alarm(0);
	if(nTimeouts==RETRANSMIT){
		printf("Error. Couldn't establish connection.\n");
      return -1;
	}
	nTimeouts=0;
  do{

    if (ret==2){//recebu uma trama de informação com Ns=0
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,0,RECEIVER);
        r=write(fd,rej_tmp,5);
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,0,RECEIVER);
        r=write(fd,rr_temp,5);
        return i-1;
      }
    }
    else if(ret==3){//recebeu uma trama de informação com Ns=1
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,1,RECEIVER);
        r=write(fd,rej_tmp,5);
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,1,RECEIVER);
        r=write(fd,rr_temp,5);
        return i-1;
      }
    }
  }while(r!=5);
}


int llclose(int fd){
  int r,rec,stateDisc=0;
  char* buf;
  int ret;
  char writer_ua[5],writer_disc[5],receiver_disc[5];
  if(MODE==WRITER){

    createDISC(writer_disc,MODE);
    do{
      r=write(fd,writer_disc,5);
      alarm(TIMEOUT);
      do{
        r=read(fd,buf,1);

        if(r==1){
          alarm(TIMEOUT);
          rec=buf[0];
          ret=validateDISC(rec,&stateDisc);
          nTimeouts=0;
        }
        else if(r==0){
          ret=0;
          if(alarm_flag==1){
            alarm_flag=0;
            nTimeouts++;
            ret=-1;
          }
        }
      }while(ret==0);
    }while( nTimeouts < RETRANSMIT && r<1);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection on closing.\n");

      return -1;
    }
    nTimeouts=0;

    createUA(writer_ua,WRITER);
    r=write(fd,writer_ua,5);
    stateDisc=0;
    return 0;
  }
  else{
    alarm(TIMEOUT);
    do{
      r=read(fd,buf,1);

      if(r==1){
        alarm(TIMEOUT);
        rec=buf[0];
        ret=validateDISC(r,&stateDisc);
        nTimeouts=0;
      }
      else{
        ret=0;
        if(alarm_flag==1){
          alarm_flag=0;
          ret=-1;
          nTimeouts++;
        }
      }
    }while(nTimeouts < RETRANSMIT && ret==0);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection on closing.\n");

      return -1;
    }
    nTimeouts=0;
    stateDisc=0;

    createDISC(receiver_disc,RECEIVER);
    r=write(fd,receiver_disc,5);
    return 0;

  }
}

int createDtPckg(unsigned char* data,unsigned long dataSz,unsigned char** pack,int n){

  int tam2 = dataSz /256;
  int tam3 = dataSz % 256;
  (*pack)=(unsigned char*) malloc(dataSz +4);
  (*pack)[0]=0x01;
  (*pack)[1]= n & 0xFF;
  (*pack)[2]=tam2 & 0xFF;
  (*pack)[3]=tam3 & 0xFF;
  int i=0;
  for(i=0;i<dataSz;i++){
    (*pack)[4+i]=data[i];
  }

  return (dataSz+4);

}

int dePkgDt(unsigned char* pckg,unsigned long pckgSz,unsigned char** data){
  (*data)=(unsigned char*)malloc(pckgSz-4);

  int i;
  for(i=0;i<pckgSz;i++){
    (*data)[i]=pckg[i+4];
  }

  return (pckgSz-4);
}
int dePkgCtrl(unsigned char* pckg,unsigned long pckgSz,int* start,unsigned long* tamanho,unsigned char** name){

  *start = (int)pckg[0];

  if(pckg[1]==0x00){
    int i=0;
    *tamanho = (pckg[3]<<24) | (pckg[4]<<16) | (pckg[5]<<8) | (pckg[6]);
    printf("tamanho = %lu\n",*tamanho);
  }
  else{
    printf("not in order\n");
  }
  int tm;
  if(pckg[7]==0x01){
    tm=(int)pckg[8];
    int j;
    (*name)= malloc(tm);
    for(j=0;j<tm;j++){
      (*name)[j]=pckg[j+9];
    }

  }
  else{
    printf("not in order\n");
  }
  return tm;
}
int createCtrlPckg(unsigned char** start,unsigned char** end,int tamanho,unsigned char* name,unsigned long nameSz){

  int sizeSz = 4;
  unsigned char bytes[sizeSz];

  bytes[0] = (tamanho >> 24) & 0xFF;
  bytes[1] = (tamanho >> 16) & 0xFF;
  bytes[2] = (tamanho >> 8) & 0xFF;
  bytes[3] = tamanho & 0xFF;

  unsigned char nameSize;
  nameSize = nameSz & 0xFF;
  unsigned char sizeSize;
  sizeSize = sizeSz & 0xFF;
  (*start) =(unsigned char*) malloc(5 + nameSz + sizeSz);
  (*end) = (unsigned char*)malloc(5 + nameSz + sizeSz);
  (*start)[0]=0x02;
  (*end)[0]=0x03;
  (*start)[1]=0x00;
  (*start)[2]=sizeSize;
  (*end)[1]=0x00;
  (*end)[2]=sizeSize;
  int i=0;
  int j=0;
  for(i = 3;i<3+sizeSz;i++){
    (*start)[i]=bytes[j];
    (*end)[i]=bytes[j];
    j++;
  }
  (*start)[3+sizeSz]=0x01;
  (*start)[4+sizeSz]=nameSize;
  (*end)[3+sizeSz]=0x01;
  (*end)[4+sizeSz]=nameSize;
  j=0;
  for(i = 5+sizeSz;i<5+sizeSz +nameSz;i++){
    (*start)[i]=name[j];
    (*end)[i]=name[j];
    j++;
  }
  return (5 + nameSz + sizeSz);
}
/*
void llwrite(int fd, unsigned char* data){
  char rec,ret;
  char buf[255];
  int r;
  int stateData=0;
  int esc=0;
  char* bcc;
  char* stuffedData[255];
  char* bccAdded[255];
  char* final[255];

  int i=0;
  int timeout=0;

  BCC2(data,bcc);
  addBcc2(data,bccAdded,bcc,255)
  stuffing(bccAdded,stuffedData,255);

    do{
    r= write(fd,buf,1);
    if(r!=0)
	{
    rec=buf[0];
    ret=validateWrt(rec,&stateData);
	}
    else if(r==0)
	{
    ret=-1;
    }
  }while(ret==0);




   do{
   if (ret==1)
  completeData(suffedData,final,0,255);
   r= read(fd,buf,5);
   if (r==0x81)
   return -1;
   else if(r==0x85)
	{
	ret=0;
	}
	else
	timeout++;
	 }
	while(timeout<3);

  do{
  if (ret==2)
  completeData(suffedData,final,0,255);
   r= read(fd,buf,5);
   if (r==0x01)
   return -1;
   else if(r==0x05)
	{
	ret=0;
	}
	else
	timeout++;
	}
	while(timeout<3);


  if(timeout==3)
  return -1;
  else
  timeout=0;

}
*/






off_t fsize(char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

int main(int argc,unsigned char** argv)
{


  // installing alarm
  struct sigaction act;
  act.sa_handler = alarmhandler;
  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGALRM, &act, NULL);

	MODE=(int)strtol(argv[2],NULL,2); //argv2 is reader 
  int fd,c;
  struct termios oldtio;
  
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1){
  perror("tcgetattr");
  exit(-1);
	}

	int r=llopen(fd,MODE);
	printf("r:%i\n",r);
	int cl=llclose(fd);
	printf("cl:%i\n",cl);
	

/*
char* file = argv[3];
//llopen
if(user == WRITER){
//abrir ficheiro
//guardar tamanho doficheiro

//fazemos o llopen
//criamso o pacote de controlo de inicio
//enviamos pelo ll write
//vamos lendo do ficheiro
//criando pacote de dado
//e enviando pelo llwrite
//chegamso ao fim do ficheiro
//mandamos o pacote de controlo de fim
}
//fazemos llclose




sleep(3);
tcsetattr(fd,TCSANOW,&oldtio);
close(fd);
*/
exit(0);
}
