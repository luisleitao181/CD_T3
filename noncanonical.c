/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res,state=0;
    struct termios oldtio,newtio;
    unsigned char buf[5], UABUF[10];

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

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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
   
        while(read(fd,buf,1)){
            //printf(":%02x:\n", buf[0]);

                switch(state){
                    case 0:
                        if(buf[0]==0x5c){
                            printf("flag RCV\n");
                            state=1;
                        }
                        else{
                        state=0;
                        }   
                        break;
                    case 1:
                        if(buf[0]==0x01){
                            printf("A RCV\n");
                            state=2;
                        }
                        else if(buf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        } 
                        break;
                    case 2:
                        if(buf[0]==0x07){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(buf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        } 
                        break;
                    case 3:
                    if(buf[0]==0x06){
                            printf("BCC RCV\n");
                            state=4;
                        }
                        else if(buf[0]==0x5c){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        }  
                        break;
                    case 4:
                        if(buf[0]==0x5c){
                            printf("last flag for SET RCV\n");
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
    printf("attempting to send UA\n\n\n");
                     UABUF[0]=0x5c;
                     UABUF[1]=0x03;
                     UABUF[2]=0x06;
                     UABUF[3]=0x05;
                     UABUF[4]=0x5c;
                  res=0; res=write(fd,UABUF,strlen(UABUF));
    
    

    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */
    //////////////////////
    ////llread
    //////////////////////
    int readstate=0,n=0;
    unsigned char readbuffer[50], message[50];
    unsigned char bcc;
    while(read(fd,readbuffer,1)){
            //printf(":%02x:\n", readbuffer[0]);

                switch(readstate){
                    case 0:
                        if(readbuffer[0]==0x5c){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        }   
                        break;
                        
                    case 1:
                        if(readbuffer[0]==0x01){
                            readstate=2;
                        }
                        else if(readbuffer[0]==0x5c){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        } 
                        break;
                        
                    case 2:
                        if(readbuffer[0]==0x07){
                            readstate=3;
                        }
                        else if(readbuffer[0]==0x5c){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        } 
                        break;
                        
                    case 3:
                        if(readbuffer[0]==0x06){
                            readstate=4;
                        }
                        else if(readbuffer[0]==0x5c){
                            
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        }  
                        break;
                   case 4:///guardar Ds e esperar pelo bcc2
                        for(int i=0; i<n ;i++){///esta a dar erro so da a ultima character
                            bcc=bcc^message[i];
                        }
                        if(readbuffer[0]==bcc){
                            readstate=5;
                            printf("message ended\n");
                        }
                        else{
                            message[n]=readbuffer[0];
                            n++;
                            printf("%c\n", readbuffer[0]);
                        }
                        break;
                   case 5://esperar pela flag
                        if(readbuffer[0]==0x5c){
                            readstate=6;
                        }
                        break;
                }   
                
                
                
    if(state==6){
    state=0; 
    break;
    }
    }
    /// if size of mesage>0 send buffer "mesage" and return
    
    
    //////////////////////
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
