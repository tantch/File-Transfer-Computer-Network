#include "application.h"
#include "link.h"
#define DATASIZE 10


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
  stuffed[r] =  data[0];
  r++;
  for(i=1;i<n-1;i++){
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
  stuffed[r]=data[n-1];
  r++;
  return r;
}

int completeData(unsigned char* data, unsigned char* final,int c, int n,unsigned char bcc2){
  int i;


  for (i=0;i<n;i++){
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
	return n+6;
}


int main(int argc,unsigned char** argv)
{

  int idN=0;
  unsigned char* buf=(unsigned char*) malloc(DATASIZE);
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

  if (fd < 0){
    perror(argv[1]); exit(-1);
  }

  if (tcgetattr(fd,&oldtio) == -1){
    perror("tcgetattr");
    exit(-1);
  }

  int r = llopen(fd,MODE);
  printf("r:%i\n",r);

  if(MODE==RECEIVER){
  	int i;
  	unsigned char* buffer;
    buffer=malloc(255);
    //first ctrl
    unsigned long fileSize;
    unsigned char* name;
    int cbyte,c,r;
    do{
      c=llread(fd,buffer);
      r=dePkgCtrl(buffer,c,&cbyte,&fileSize,&name);
    }while(cbyte!=2);
    printf("File name:%s\nfile size:%lu\n",name,fileSize);
    int counter=(int)fileSize;
    unsigned char * data;
    const char* fmode="wb";
    int f=open_file("pinguim2.gif",fmode);

    while(counter>0){
      printf("Couter:%i\n",counter);
     c=llread(fd,buffer);
     r= dePkgDt(buffer,c,&data);
     fwrite(data, sizeof(char), r, FINFO.f);
     //TODO se sucesso
     counter-=r;
    }
  	//printChar(buffer,c);
  }
  else if(MODE==WRITER){
    const char* fpath="pinguim.gif";
    int tm= strlen(fpath);
    printf("Name size :%i\n",tm);
    const char* fmode="rb";
    int f=open_file(fpath,fmode);
    printf("file opened with f:%i\n",f);
    int fs=getFileSize();
    printf("fs:%i",fs);
    printf("file size : %lu\n",FINFO.size);
    //open do ficheiro
    //guardar o tamanho do ficheiro

    char *startCtrl,*endCtrl;
    int re =createCtrlPckg(&startCtrl,&endCtrl,FINFO.size,fpath,tm);
    int p=llwrite(fd,startCtrl,re);
    int k=0;

    unsigned char* pack;
    int counter=FINFO.size;
    while(counter>0){
      printf("Couter:%i\n",counter);
      int tam=fread(buf, sizeof(char), DATASIZE, FINFO.f);
      int ri=createDtPckg(buf,tam,&pack,idN);
      idN++;
      int p=llwrite(fd,pack,ri);
      counter-=tam;
    }
    //p=llwrite(fd,endCtrl,re);

  }
char lixo[255];
	read(fd,lixo,255);
  int cl=llclose(fd);
  printf("cl:%i\n",cl);

  sleep(3);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  exit(0);
}
