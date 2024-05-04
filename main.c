#include "llfunctions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res,state=0,saiwhile=0;
    struct termios oldtio,newtio;
    unsigned char buf[7],rcvbuf[10];
    int i, sum = 0, speed = 0;

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    const char *serialPort = argv[1];
    const char *role = argv[2];
    const char *filename = argv[3];


    
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    Role role;

  if(!strcmp("t",role))
  {
    role = transmitter;                    //nao sei se isto e assim
    //printf("Role: TRANSMITER\n");
  }
  else if(!strcmp("r",role))
  {
    role = reciever;
    //printf("Role: RECEIVER\n");
  }
  LinkLayer params;
  params.role = role;
  strcpy(params.serialPort,serialPort);

  int fdsize=llopen(params)
  if(fd<0)exit(-1);



///////////////////////////////////////////////////////////////////////////////////////////////////////////

  if(role == transmitter)
  {
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  else if(role == reciever)
  {
  }

  return 0;
}
