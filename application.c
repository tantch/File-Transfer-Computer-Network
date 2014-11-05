#include "application.h"



int createDtPckg(unsigned char* data,unsigned long dataSz,unsigned char** pack,int n){

  int tam2 = dataSz / 256;
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

int dePkgDt(unsigned char* pckg,unsigned long pckgSz,unsigned char** data,int * NC){
  //(*data)=(unsigned char*)malloc(pckgSz-4);

  int i;
  //if(pckg[0]!=0x01)
    //return-1;
    (*NC)=pckg[1];
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
  }
  else{
    printf("not in order\n");
    return -1;
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
    return -1;
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
int aplRead(int fd){
  int idN=0;
  unsigned char* buf=(unsigned char*) malloc(AINFO.maxSize);
  int NC=0;
  int i;
  unsigned char* buffer;
  buffer=malloc(255);
  //first ctrl
  unsigned long fileSize;
  unsigned char* name;
  int cbyte,c,r;
  r = llopen(fd,MODE);
  if(r<0){
    printf("Failed to open connection\n");
    return -1;
  }

  do{
    c=llread(fd,buffer);
  } while(c<0);
  r=dePkgCtrl(buffer,c,&cbyte,&fileSize,&name);

  if(cbyte !=2){
    printf("Error: first pakage was not start control package\n");
    return -1;
  }
  if(verbose)printf("File name:%s\nfile size:%lu\n",name,fileSize);
  int counter=(int)fileSize;
  unsigned char * data=(unsigned char*)malloc(100-4);
  const char* fmode="wb";
  int f=open_file(name,fmode);
  if(f==0){
  printf("Error opening file\n");
    return -1;
  }
  int n;
  int re=0;
  while(counter>0){
    if(verbose)printf("Counter:%i\n",counter);
    do{
      c=llread(fd,buffer);
    }while(c<0);

    re= dePkgDt(buffer,c,&data,&n);
    counter-=re;
    fwrite(data, sizeof(char), re, FINFO.f);
  }
  char lixo[255];
  read(fd,lixo,255);


  do{
    c=llread(fd,buffer);
  } while(c<0);
  r=dePkgCtrl(buffer,c,&cbyte,&fileSize,&name);
  int cl=llclose(fd);
  if(cl<0){
    printf("Error closing connection\n");
    return -1;
  }
  return 0;
}
int aplWrite(int fd,char* fileName){
  int idN=0;
  unsigned char* buf=(unsigned char*) malloc(AINFO.maxSize);
  const char* fpath=fileName;
  int tm= strlen(fpath);
  const char* fmode="rb";
  int f=open_file(fpath,fmode);
  if(f==0){
    printf("Failed to open file\n");
    return -1;
  }
  int fs=getFileSize();
  if(verbose)printf("file size : %lu\n",FINFO.size);
  //open do ficheiro
  //guardar o tamanho do ficheiro
  int r = llopen(fd,MODE);
  if(r<0){
    printf("Failed to open connection\n");
    return -1;
  }

  char *startCtrl,*endCtrl;
  int re =createCtrlPckg(&startCtrl,&endCtrl,FINFO.size,fpath,tm);
  int p;
  do{
    p=llwrite(fd,startCtrl,re);
  }while(p==-1);
  int k=0;
  unsigned char* pack;
  int counter=FINFO.size;
  while(counter>0){
    if(verbose)printf("Counter1:%i\n",counter);
    int tam=fread(buf, sizeof(char), AINFO.maxSize, FINFO.f);
    int ri=createDtPckg(buf,tam,&pack,idN);
    idN++;
    do{
      p=llwrite(fd,pack,ri);
    }while(p==-1);
    if(p==-2){
      if(verbose)printf("Timed out \n");
      return -1;
    }
    counter-=tam;
  }
  do{
    p=llwrite(fd,endCtrl,re);
  }while(p==-1);
  int cl=llclose(fd);
  if(cl<0){
    printf("Failed to close connection\n");
    return -1;
  }
  return 0;
}
