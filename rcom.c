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

}



int main(int argc,unsigned char** argv)
{


  // installing alarm
  verbose=1;
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

  if(argc>4){
    AINFO.filename=argv[4];
    if(verbose)printf("%s\n",argv[4]);
  }
  else{
    AINFO.filename="pinguim.gif";
  }
  if(argc>3){
    AINFO.maxSize=atoi(argv[3]) -4;
    if(verbose)printf("%x\n",atoi(argv[3]));
  }
  else{
    AINFO.maxSize=100;
  }


  if(MODE==RECEIVER){
    aplRead(fd);

  }
  else if(MODE==WRITER){
    aplWrite(fd,AINFO.filename);

  }



  sleep(3);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  exit(0);
}
