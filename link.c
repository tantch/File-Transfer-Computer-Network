#include "link.h"



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
  (*data)=(unsigned char*)malloc(pckgSz-4);

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



off_t fsize(char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}
