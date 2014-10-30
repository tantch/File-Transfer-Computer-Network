#include "application.h"
#include "link.h"



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

  unsigned long datasize=100;
  int idN=0;

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
  	
    int c=llread(fd,buffer);
  	printChar(buffer,c);	
  }
  else if(MODE==WRITER){

    //open do ficheiro
    //guardar o tamanho do ficheiro
    
    char* cenas = "ola a todos"; //em vez dsito
    unsigned long tam=11;
    char* nome="ola.txt";
    unsigned long tm=7;
	printf("Pure data size = %lu \n",tam);
    char *startCtrl,*endCtrl;
    int re =createCtrlPckg(&startCtrl,&endCtrl,tam,nome,tm);
    //int p=llwrite(fd,startCtrl,re);
    int nData =(int) tm/datasize +1;
    int k=0;
   
    unsigned char* pack;
    int ri=createDtPckg(cenas,tam,&pack,idN);
    idN++;
    int p=llwrite(fd,pack,ri);
    
  }
  int cl=llclose(fd);
  printf("cl:%i\n",cl);

  sleep(3);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  exit(0);
}
