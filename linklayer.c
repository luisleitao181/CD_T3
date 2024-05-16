/*FUNCTIONS*/
#include "linklayer.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
//#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
int fd;
struct termios oldtio, newtio;
linkLayer connectionParameters;

#define SET 0x07 
#define UA 0x06
#define DISC 0x0A
#define FLAG 0x5C
#define A_Tx 0x01 
#define A_Rx 0x03 


int llopen(linkLayer linklayer)
{
    strcpy(connectionParameters.serialPort, linklayer.serialPort);
    connectionParameters.role = linklayer.role;
    connectionParameters.baudRate = linklayer.baudRate;
    connectionParameters.nRetransmissions = linklayer.nRetransmissions;
    connectionParameters.timeOut = linklayer.timeOut;

    char* serialPort = connectionParameters.serialPort;

    fd = open(serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPort);
        return -1;
    }

    // save settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        return -1;
    }

    // clear struct
    memset(&newtio, 0, sizeof(newtio));

    // populate new port settings
    newtio.c_cflag = 9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // No minimum bytes
    //(non-canonical)Read returns immediately with as many characters available in the queue

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    //printf("New termios structure set\n");

    unsigned char buf[Max_Size], rcvbuf[Max_Size];
    unsigned char UABUF[Max_Size];


    //enum OpenStates state = OPEN_START;
    int state=0;
    if(connectionParameters.role==TRANSMITTER){
    buf[0]=FLAG;
    // buf[1]=0x38;///random (to test state machine)
    // buf[2]=0x5c;
    buf[1]=A_Tx;
    buf[2]=SET;
    buf[3]=A_Tx ^ SET;
    buf[4]=FLAG;

    // for(int i=0;i<(strlen(buf)-1);i++){
    //     printf("%02x\n",buf[i]);
    // }
    int res = write(fd,buf,strlen(buf));
    printf("%d bytes written\n\n\n", res);

    while(read(fd,rcvbuf,1)){
        printf("%02x\n",rcvbuf[0]);
        switch(state){
                    case 0:
                        if(rcvbuf[0]==FLAG){
                             printf("flag RCV\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }   
                        break;
                    case 1:
                        if(rcvbuf[0]==A_Rx){
                             printf("A RCV\n");
                            state=2;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 2:
                        if(rcvbuf[0]==SET){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        } 
                        else{
                            state=0;
                        } 
                        break;
                    case 3:
                        if(rcvbuf[0]==A_Rx ^ SET){
                            printf("BCC RCV\n");
                            state=4;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 4:
                        if(rcvbuf[0]==FLAG){
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
    if(connectionParameters.role==RECEIVER){
        while(read(fd,buf,1)){
            printf(":%02x:\n", buf[0]);
                switch(state){
                    case 0:
                        if(buf[0]==FLAG){
                            printf("flag RCV\n");
                            state=1;
                        }
                        else{
                        state=0;
                        }   
                        break;
                    case 1:
                        if(buf[0]==A_Tx){
                            printf("A RCV\n");
                            state=2;
                        }
                        else if(buf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        } 
                        break;
                    case 2:
                        if(buf[0]==SET){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(buf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        } 
                        break;
                    case 3:
                    if(buf[0]==A_Tx ^ SET){
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
                     UABUF[0]=FLAG;
                     UABUF[1]=A_Rx;
                     UABUF[2]=UA;
                     UABUF[3]=A_Rx ^ UA;
                     UABUF[4]=FLAG;
                  res=0; res=write(fd,UABUF,strlen(UABUF));
}
}
  return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llclose(linkLayer connectionParameters, int showStatistics)
{
    int state=0;
    unsigned char buf[Max_Size], rcvbuf[Max_Size];
    unsigned char DISCBUF[Max_Size];
    unsigned char UABUF[Max_Size];


    if(connectionParameters.role==TRANSMITTER){                                                   
    buf[0]=FLAG;
    buf[1]=A_Tx;
    buf[2]=DISC;
    buf[3]=A_Tx ^ DISC;
    buf[4]=FLAG;

    int res = write(fd,buf,strlen(buf));
    printf("%d bytes written\n\n\n", res);

    while(read(fd,rcvbuf,1)){
        printf("%02x\n",rcvbuf[0]);
        switch(state){
                    case 0:
                        if(rcvbuf[0]==FLAG){              
                            printf("flag RCV\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }   
                        break;
                    case 1:
                        if(rcvbuf[0]==A_Rx){
                            printf("A RCV\n");
                            state=2;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 2:
                        if(rcvbuf[0]==DISC){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        } 
                        else{
                            state=0;
                        } 
                        break;
                    case 3:
                        if(rcvbuf[0]==A_Rx ^ DISC){
                            printf("BCC RCV\n");
                            state=4;
                        }
                        else if(rcvbuf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 4:
                        if(rcvbuf[0]==FLAG){
                            printf("DISC RCV\n");
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
        UABUF[0]=FLAG;
        UABUF[1]=A_Tx;
        UABUF[2]=UA;
        UABUF[3]=A_Tx ^ UA;
        UABUF[4]=FLAG;
        res=0; res=write(fd,UABUF,strlen(UABUF));

    }
    /////
    if(connectionParameters.role==RECEIVER){
        while(read(fd,buf,1)){
            printf(":%02x:\n", buf[0]);
            switch(state){
                case 0:
                    if(buf[0]==FLAG){
                        printf("flag RCV\n");
                        state=1;
                    }
                    else{
                    state=0;
                    }   
                    break;
                case 1:
                    if(buf[0]==A_Tx){
                        printf("A RCV\n");
                        state=2;
                        }
                        else if(buf[0]==FLAG){
                        printf("flag RCV (went back)\n");
                        state=1;
                    }
                    else{
                    state=0;
                    } 
                    break;
                case 2:
                    if(buf[0]==DISC){
                        printf("C RCV\n");
                        state=3;
                    }
                    else if(buf[0]==FLAG){
                        printf("flag RCV (went back)\n");
                        state=1;
                    }
                    else{
                        state=0;
                    } 
                    break;
                case 3:
                    if(buf[0]==A_Tx ^ DISC){
                        printf("BCC RCV\n");
                        state=4;
                    }
                    else if(buf[0]==FLAG){
                        printf("flag RCV (went back)\n");
                        state=1;
                    }
                    else{
                        state=0;
                    }  
                    break;
                case 4:
                    if(buf[0]==FLAG){
                        printf("last flag for DISC RCV\n");
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
        printf("attempting to send DISC\n");
        DISCBUF[0]=FLAG;
        DISCBUF[1]=A_Rx;
        DISCBUF[2]=DISC;
        DISCBUF[3]=A_Rx ^ DISC;
        DISCBUF[4]=FLAG;
        int res=0; res=write(fd,DISCBUF,strlen(DISCBUF));
        
        while(read(fd,buf,1)){
            switch (state)
                    {
                    case 0:
                        if(buf[0]==FLAG){
                            printf("flag RCV\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }   
                        break;
                    case 1:
                        if(buf[0]==A_Tx){
                            printf("A RCV\n");
                            state=2;
                        }
                        else if(buf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                        state=0;
                        } 
                        break;
                    case 2:
                        if(buf[0]==UA){
                            printf("C RCV\n");
                            state=3;
                        }
                        else if(buf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        } 
                        break;
                    case 3:
                        if(buf[0]==A_Tx ^ UA){
                            printf("BCC RCV\n");
                            state=4;
                        }
                        else if(buf[0]==FLAG){
                            printf("flag RCV (went back)\n");
                            state=1;
                        }
                        else{
                            state=0;
                        }  
                        break;
                    case 4:
                        if(buf[0]==FLAG){
                        printf("last flag for DISC RCV\n");
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
        printf("DISCONNECTING\n");

        if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
        {
            perror("tcsetattr");
            return -1;
        }

        close(fd);
    }
    
    
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llread(char* packet)
{
    int readstate=0,n=0;
    unsigned char readbuffer[50], message[50];
    unsigned char bcc;
    while(read(fd,readbuffer,1)){
            //printf(":%02x:\n", readbuffer[0]);

                switch(readstate){
                    case 0:
                        if(readbuffer[0]==FLAG){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        }   
                        break;
                        
                    case 1:
                        if(readbuffer[0]==A_Tx){
                            readstate=2;
                        }
                        else if(readbuffer[0]==FLAG){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        } 
                        break;
                        
                    case 2:
                        if(readbuffer[0]==SET){
                            readstate=3;
                        }
                        else if(readbuffer[0]==FLAG){
                            readstate=1;
                        }
                        else{
                        readstate=0;
                        } 
                        break;
                        
                    case 3:
                        if(readbuffer[0]==A_Tx ^ SET){
                            readstate=4;
                        }
                        else if(readbuffer[0]==FLAG){
                            
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
                        if(readbuffer[0]==FLAG){
                            readstate=6;
                        }
                        break;
                }   
    if(readstate==6){
    readstate=0; 
    break;
    }
    }
    /// if size of mesage>0 send buffer "mesage" and return
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llwrite(char* buf, int bufSize)
{
   /////so teste
    buf[0]=FLAG;
    buf[1]=A_Tx;
    buf[2]=SET;
    buf[3]=A_Tx ^ SET;
    buf[4]='o';
    buf[5]='l';
    buf[6]='a';
    buf[7]=FLAG;
    int res = write(fd,buf,strlen(buf));
    ////////
}
