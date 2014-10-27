#include "validation.h"


/*
*@param data byte a comparar para alterar a M.E.
*@param stateRRJ estado da maquina
*@ret int -1->erro(mandar tudo abaixo)
*     0-> continuar
*     1-> recebido trama de confirmaçao RR com N(r)=0
*     2-> recebido trama de confirmação RR com N(r)=1
*     3-> recebido trama de rejeição REJ com N(r)=0
*     4-> recebido trama de rejeição REJ com N(r)=1
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
  	}
    else if(data==CREJ0){
      *stateRRJ=8;
    }
    else if(data==CREJ1){
      *stateRRJ=10;
    }
    else{
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
      *stateRRJ=12;
      return 2;
    }else{
      *stateRRJ=0;
      return 0;
    }

    case 8:
    if(data==F){
    	*stateRRJ=1;
    }
    else if(data==A1^CREJ0){
    	*stateRRJ=10;
    }
    else{
    	*stateRRJ=0;
    	return 0;
    }

    case 9:
    if(data==F){
    	*stateRRJ=12;
    	return 3;
    }
    else{
    	*stateRRJ=0;
    	return 0;
    }

    case 10:
    if(data==F){
    	*stateRRJ=1;
    }
    else if(data==A1^CREJ1){
    	*stateRRJ=11;
    }
    else{
    	*stateRRJ=0;
    	return 0;
    }

    case 11:
    if(data==F){
    	*stateRRJ=12;
    	return 4;
    }
    else{
    	*stateRRJ=0;
    	return 0;
    }
    
    case 12:
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
    }else if(disc == (A1 ^ CDISC) || disc == (A0 ^ CDISC)){
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
