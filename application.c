#include "application.h"

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
  char rec;
  int ret=0;	
  char buf[255];
  int r;
  int stateData=0;
  char bccData=0x00;
  int esc=0;
  int i=0;

  printf("a começar a ler...\n");

  do{
    r = read(fd,buf,1);
    if(r==1){
      rec=buf[0];
		if(rec!= 0x7e && (stateData==7 || stateData==10)){
        destuffing(rec,buffer,&bccData, &esc,&i);
      }
      ret=validateRcv(rec,&stateData);
	    printf("read byte 0x%x\n",rec);
      printf("state:%i\n",stateData);
      
    }
  }while(ret==0);
printf("bcc is equal to: 0x%x\n",bccData);

  do{

    if (ret==2){//recebu uma trama de informação com Ns=0
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,0,RECEIVER);
        r=write(fd,rej_tmp,5);
        printf("\nreceiver sending REJ Ns=0: ");
        printChar(rej_tmp,5);
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,0,RECEIVER);
        r=write(fd,rr_temp,5);
        printf("\nreceiver sending RR Ns=0:");
        printChar(rr_temp,5); 
        return i-1;
      }
    }
    else if(ret==3){//recebeu uma trama de informação com Ns=1
      if(bccData!=0){
        char rej_tmp[5];
        createREJ(rej_tmp,1,RECEIVER);
        r=write(fd,rej_tmp,5);
        printf("\nreceiver sending REJ Ns=1: ");
        printChar(rej_tmp,5);
        return -1;
      }
      else{
        char rr_temp[5];
        createRR(rr_temp,1,RECEIVER);
        r=write(fd,rr_temp,5);
        printf("\nreceiver sending RR Ns=1: ");
        printChar(rr_temp,5);
        return i-1;
      }
    }
  }while(r!=5);
}

int llwrite(int fd, unsigned char* data,int tm){
	printf("starting llwrite\n");
  char rec,ret;
  char buf[255];
  int r, tm2,tm3;
  int stateRRJ=0;
  int esc=0;
  char* bcc;
  char* stuffedData;
  char* final;

  int i=0;
  int timeout=0;
  BCC2(data,&bcc,tm);
  printf("bcc calculated is 0x%x\n",bcc);
  final=(unsigned char*)malloc(tm+5);
  tm2=completeData(data,final,Ns,tm,bcc);
  stuffedData =(unsigned char *)malloc(tm2*2);
  tm3=stuffing(final,stuffedData,tm2);
	int p=0;

  printf("************\nDATA:\n");
  printChar(data,tm);
  printf("************\nFINAL DATA:\n");
  printChar(final,tm2);
  printf("************\nSTUFFED DATA:\n");
  printChar(stuffedData,tm3);



  do{
    r= write(fd,stuffedData,tm3);
    alarm(TIMEOUT);
    do{
      p=read(fd,buf,1);
      if(p==1){
        alarm(TIMEOUT);
        rec=buf[0];
        ret=validateRRJ(rec,&stateRRJ);

      }
      else if(p==0){
        ret=0;
        if(alarm_flag==1){
          alarm_flag=0;
          ret=-1;
          nTimeouts++;
          alarm(TIMEOUT);
        }
      }

    }while(ret==0);
  }while(nTimeouts<RETRANSMIT && p<1);
	nTimeouts=0;
	alarm(0);

  if(ret==1){
    Ns=0;
    return 0;
  }
  else if(ret==2){
    Ns=1;
    return 0;
  }
  else if(ret==3 || ret==4){
    return -1;
  }
  return 0;
}

int llclose(int fd){
printf("entering close\n");
  //config(3,0,fd);
  int r,rec,stateDisc=0,stateUA=0;
  char* buf;
	buf=malloc(255);
  int ret=0;
  char writer_ua[5],writer_disc[5],receiver_disc[5];
  if(MODE==WRITER){

    createDISC(writer_disc,MODE);
    do{
      r=write(fd,writer_disc,5);
		printf("wrote %i bytes\n",r);
      alarm(TIMEOUT);
      do{
        r=read(fd,buf,1);

        if(r==1){
          alarm(TIMEOUT);
          rec=buf[0];
          ret=validateDISC(rec,&stateDisc);
          printf("writer state:0x%x\n",stateDisc);
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
      printf("ret:%i\n",ret);
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
	printf("receiver\n");
    alarm(TIMEOUT);
    do{
      r=read(fd,buf,1);
		perror("error :");
      if(r==1){
        alarm(TIMEOUT);
        rec=buf[0];
        ret=validateDISC(rec,&stateDisc);
		printf("state after validate:%i\n",stateDisc);
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
