/*Non-Canonical Input Processing*/

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


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");


    buf[0]=0x5c;
    buf[1]=0x38;
    buf[2]=0x5c;
    buf[3]=0x01;
    buf[4]=0x07;
    buf[5]=0x06;
    buf[6]=0x5c;

    for(int i=0;i<(strlen(buf)-1);i++){
        printf("%02x\n",buf[i]);
    }
    res = write(fd,buf,strlen(buf));
    printf("%d bytes written\n\n\n", res);

    while(read(fd,rcvbuf,1)){
        printf("%02x\n",rcvbuf[0]);
        switch(state){
                    case 0:
                        if(rcvbuf[0]==0x5c){
                            printf("flag RCV\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }   
                        break;
                    case 1:
                        if(rcvbuf[0]==0x03){
                            printf("A RCV\n");
                            state=2;
                        }
                        else if(rcvbuf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 2:
                        if(rcvbuf[0]==0x06){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(rcvbuf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        } 
                        else{
                            state=0;
                        } 
                        break;
                    case 3:
                        if(rcvbuf[0]==0x05){
                            printf("BCC RCV\n");
                            state=4;
                        }
                        else if(rcvbuf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 4:
                        if(rcvbuf[0]==0x5c){
                            printf("UA RCV\n");
                            state=5;
                        }
                        else{
                            state=0;
                        }
                        break;
                }
    if(state==5){
    state=0;
    break;
}
    
    }
    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }


    close(fd);
    return 0;
}
