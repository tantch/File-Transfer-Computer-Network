#include "general.h"


nTimeouts=0;
alarm_flag=0;
MODE=0;
Ns=0;

void printChar(unsigned char* cena,int tam){
  int i;
  for(i=0; i<tam;i++){
    if(verbose)printf("char[%i]:0x%x\n",i,cena[i]);
  }
}

void alarmhandler(int signo) {

  if(verbose)printf("alarm handler\n");
  alarm_flag=1;

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

int open_file(const char* fpath, char* mode){

  FILE * f = fopen( fpath, mode );
  if (f<=0) return 0;
  else {
    FINFO.f=f;
    return 1;
  }

}

int getFileSize(){

  int size = -1;
  if( fseek(FINFO.f, 0L, SEEK_END) < 0) {
    return -1;
  }

  if( (size = ftell(FINFO.f)) < 0){
    return -1;
  }
  if(fseek(FINFO.f, 0, SEEK_SET)<0){
    return -1;
  }
  FINFO.size = size;

  return 1;


}
