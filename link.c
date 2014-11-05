#include "link.h"


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
      if(res==1){
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
    }while(ret==0 && nTimeouts<RETRANSMIT);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection.\n");
      nTimeouts=0;
      return -1;
    }

    res=write(fd,ua,5);
    nTimeouts=0;
    return 0;
  }

  else{

    unsigned char set[5];
    createSET(set,mode);
    do{
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
    }while(nTimeouts<RETRANSMIT && res<1);

    if(nTimeouts==RETRANSMIT){
    nTimeouts=0;
    alarm(0);
      printf("Error. Couldn't establish connection.\n");
      return -1;
    }

    alarm(0);
    nTimeouts=0;
    return 0;

  }
}

int llread(int fd, char * buffer){
  unsigned char rec;
  int ret=0;
  unsigned char buf[1];
  int r;
  int stateData=0;
  char bccData=0x00;
  int esc=0;
  int i=0;


  do{

    r = read(fd,buf,1);
    if(r==1){
      rec=buf[0];
    if(rec!= 0x7e && (stateData==7 || stateData==10)){
        destuffing(rec,buffer,&bccData, &esc,&i);
      }
      if(stateData==1){
         bccData=0x00;
      }
      ret=validateRcv(rec,&stateData);

    }
  }while(ret==0);


  do{

    if (ret==2){//recebu uma trama de informa��o com Ns=0
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,0,RECEIVER);
        r=write(fd,rej_tmp,5);
        if(verbose)printf("\nreceiver sending REJ Ns=0: ");
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,0,RECEIVER);
        r=write(fd,rr_temp,5);
        //if(verbose)printf("\nreceiver sending RR Ns=0:");
        return i-1;
      }
    }
    else if(ret==3){//recebeu uma trama de informa��o com Ns=1
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,1,RECEIVER);
        r=write(fd,rej_tmp,5);
        if(verbose)printf("\nreceiver sending REJ Ns=1: ");
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,1,RECEIVER);
        r=write(fd,rr_temp,5);
        //if(verbose)printf("\nreceiver sending RR Ns=1: ");
        return i-1;
      }
    }
  }while(r!=5);
}

int llwrite(int fd, unsigned char* data,int tm){
  char rec,ret;
  char buf[255];
  int r, tm2,tm3;
  int stateRRJ=0;
  int esc=0;
  unsigned char bcc=0x00;
  char* stuffedData;
  char* final;

  int i=0;
  int timeout=0;
  BCC2(data,&bcc,tm);
  final=(unsigned char*)malloc(tm+5);
  tm2=completeData(data,final,Ns,tm,bcc);
  stuffedData =(unsigned char *)malloc(AINFO.maxSize*2);
  tm3=stuffing(final,stuffedData,tm2);
  int p=0;

  do{
    stateRRJ=0;
	int missing=tm3;
	do{	
   		r= write(fd,stuffedData,tm3);
		stuffedData+=r;
		missing-=r;
	}while(missing>0);
	stuffedData-=tm3;
    alarm(TIMEOUT);
    do{
      p=read(fd,buf,1);
      if(p==-1){
        perror("Error:");
        ret=-1;
      }
      if(p==1){
        alarm(TIMEOUT);
        rec=buf[0];
        ret=validateRRJ(rec,&stateRRJ);
      }
      else{
        ret=0;
        if(alarm_flag==1){
          alarm_flag=0;
          ret=-1;
          nTimeouts++;
          alarm(TIMEOUT);
        }
      }

    }while(ret==0);
    if(verbose)printf("ntimeouts:%i\n",nTimeouts);
  }while(nTimeouts<RETRANSMIT && p<1);
  if(nTimeouts==RETRANSMIT){
    nTimeouts=0;
    alarm(0);
    return -2;
  }
  nTimeouts=0;
  alarm(0);

  if(ret==1){
    Ns=1;
    return 0;
  }
  else if(ret==2){
    Ns=0;
    return 0;
  }
  else if(ret==3 || ret==4){
    return -1;
  }
  return 0;
}

int llclose(int fd){
  int r,rec,stateDisc=0,stateUA=0;
  char buf[255];
  int ret=0;
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
    }while( nTimeouts < RETRANSMIT && ret<1);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection on closing.\n");
      nTimeouts=0;
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
        ret=validateDISC(rec,&stateDisc);

        nTimeouts=0;
      }
      else{
        ret=0;
        if(alarm_flag==1){
          alarm_flag=0;
          nTimeouts++;
  alarm(TIMEOUT);
        }
      }

    }while(nTimeouts < RETRANSMIT && ret<1);
    alarm(0);
    if(nTimeouts==RETRANSMIT){
      printf("Error. Couldn't establish connection on closing.\n");
  nTimeouts=0;
      return -1;
    }
    nTimeouts=0;
    stateDisc=0;
    ret=-1;
    createDISC(receiver_disc,RECEIVER);
    r=write(fd,receiver_disc,5);
    do{
      r=read(fd,buf,1);
      alarm(TIMEOUT);

      if(r==1){
        rec=buf[0];
        ret=validateUA(rec,&stateUA);
        alarm_flag=0;
        nTimeouts=0;
      }
      else if(r==0){

        if(alarm_flag==1){
          alarm_flag=0;
          nTimeouts++;
          alarm(TIMEOUT);
        }
      }
    }while(nTimeouts <= RETRANSMIT && r==0);
  alarm(0);
    if(nTimeouts==RETRANSMIT){
    printf("Error. Couldn't establish connection on closing.\n");
  nTimeouts=0;
      return -1;
      }
  nTimeouts=0;
    }
}
void BCC2(unsigned char* data, unsigned char* final, int n){
  int i;
  for (i=0;i<n;i++){
    if(verbose)printf("calculaing xor of 0x%x and 0x%x\n",(*final),data[i]);
    (*final)=data[i]^(*final);
  }
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
