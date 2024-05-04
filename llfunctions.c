/*FUNCTIONS*/
#include "llfunctions.h"

#include <string.h>
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
int fd;
linklayer newll;

int llopen(linklayer linklayer)
{
   strcpy(newll.serialPort, linklayer.serialPort);
    newll.role = linklayer.role; 

    char* serialPort = newll.serialPort;

    fd = open(serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPort);
        exit(-1);
    }

    // save settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // clear struct
    memset(&newtio, 0, sizeof(newtio));

    // populate new port settings
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // No minimum bytes
    //(non-canonical)Read returns immediately with as many characters available in the queue

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    //printf("New termios structure set\n");

    unsigned char buf[Max_Size];
    unsigned char response[Max_Size];

    char* readChar;
    readChar = malloc(sizeof(char));

    enum OpenStates state = OPEN_START;

    if(newll.role==transmitter){
    buf[0]=0x5c;
    buf[1]=0x38;///random (to test)
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
    if(newll==receiver){
        while(read(fd,buf,1)){
            printf(":%02x:\n", buf[0]);
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
    printf("attempting to send UA\n");
                     UABUF[0]=0x5c;
                     UABUF[1]=0x03;
                     UABUF[2]=0x06;
                     UABUF[3]=0x05;
                     UABUF[4]=0x5c;
                  res=0; res=write(fd,UABUF,strlen(UABUF));
}
}
  return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llclose()
{
    if(linklayer==transmitter){                                                                                         ////////////ainda so esta o codigo de llopen tem que se mudar para os valores do close
    buf[0]=0x5c;
    buf[1]=0x38;
    buf[2]=0x5c;
    buf[3]=0x01;
    buf[4]=0x07;
    buf[5]=0x06;
    buf[6]=0x5c;

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
    }
    /////
    if(linklayer==reciever){
        while(read(fd,buf,1)){
            printf(":%02x:\n", buf[0]);
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
        printf("attempting to send UA\n");
        UABUF[0]=0x5c;
        UABUF[1]=0x03;
        UABUF[2]=0x06;
        UABUF[3]=0x05;
        UABUF[4]=0x5c;
        res=0; res=write(fd,UABUF,strlen(UABUF));
    }
    
    
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llread()
{
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
                        for(int i=0; i<n ;i++){                                                       ///esta a dar erro so da a ultima character
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
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llwrite()
{
    /////so teste
    buf[0]=0x5c;
    buf[1]=0x01;
    buf[2]=0x07;
    buf[3]=0x06;
    buf[4]='o';
    buf[5]='l';
    buf[6]='a';
    buf[7]=0x5c;
    res = write(fd,buf,strlen(buf));
    ////////
}
