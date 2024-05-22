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
linkLayer connectionParametersllclose;

#define SET 0x07 
#define UA 0x06
#define DISC 0x0A
#define FLAG 0x5C
#define A_Tx 0x01 
#define A_Rx 0x03 

#define RR(n) (0x01 | (n<<4)) //000n0001
#define REJ(n) (0x05 | (n<<4))//000n0101
#define CONTROLFIELD_S(n) (0x08 | n<<6)////1n000000

#define ESCAPE_OCTET 0x5D
#define XOR_OCTET 0x20



// //alarm temos que testar que nao faco a menor ideia se tenho isto do alarm minimamente correta
// ///////
// int alarmEnabled = FALSE;
// int alarmCount = 0;

// void alarmHandler(int signal)
// {
//     alarmEnabled = FALSE;
//     alarmCount++;
// }



int llopen(linkLayer linklayer)
{
    strcpy(connectionParameters.serialPort, linklayer.serialPort);
    connectionParameters.role = linklayer.role;
    connectionParameters.baudRate = linklayer.baudRate;
    connectionParameters.nRetransmissions = linklayer.nRetransmissions;
    connectionParameters.timeOut = linklayer.timeOut;


    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
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
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
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
        buf[1]=A_Tx;
        buf[2]=SET;
        buf[3]=A_Tx ^ SET;
        buf[4]=FLAG;


        int res = write(fd,buf,strlen(buf));
        sleep(1);
        printf("%d bytes written\n\n\n", res);

        while(state!=5){
            read(fd,rcvbuf,1);
            //printf("%02x\n",rcvbuf[0]);
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
        }
        state=0;
    }
    if(connectionParameters.role==RECEIVER){
        while(state!=5){
            read(fd,buf,1);
            //printf(":%02x:\n", buf[0]);
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
                        printf("last flag for SET RCV\n");
                        state=5;                           
                    }
                    else{
                        state=0;
                    }
                    break;
                }
        }
        printf("attempting to send UA\n");
        UABUF[0]=FLAG;
        UABUF[1]=A_Rx;
        UABUF[2]=UA;
        UABUF[3]=A_Rx ^ UA;
        UABUF[4]=FLAG;
        res=0;
        res=write(fd,UABUF,strlen(UABUF));
        sleep(1);
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llclose(linkLayer linklayer, int showStatistics)
{
    int state=0;
    unsigned char buf[Max_Size], rcvbuf[Max_Size];
    unsigned char DISCBUF[Max_Size];
    unsigned char UABUF[Max_Size];

    connectionParametersllclose.role = linklayer.role;



    if(connectionParametersllclose.role==TRANSMITTER){                                                   
        buf[0]=FLAG;
        buf[1]=A_Tx;
        buf[2]=DISC;
        buf[3]=A_Tx ^ DISC;
        buf[4]=FLAG;

        int res = write(fd,buf,strlen(buf));
        sleep(1);

        printf("%d bytes written\n\n\n", res);

        while(state!=5){
            read(fd,rcvbuf,1);
            //printf("%02x\n",rcvbuf[0]);
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
        }
        state=0;
        printf("attempting to send UA\n");
        UABUF[0]=FLAG;
        UABUF[1]=A_Tx;
        UABUF[2]=UA;
        UABUF[3]=A_Tx ^ UA;
        UABUF[4]=FLAG;
        res=0; 
        res=write(fd,UABUF,strlen(UABUF));
        sleep(1);
    }
    /////
    if(connectionParametersllclose.role==RECEIVER){

        while(state!=5){
            read(fd,buf,1);
            //printf(":%02x:\n", buf[0]);
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
            
        }
        state=0;
        printf("attempting to send DISC\n");
        DISCBUF[0]=FLAG;
        DISCBUF[1]=A_Rx;
        DISCBUF[2]=DISC;
        DISCBUF[3]=A_Rx ^ DISC;
        DISCBUF[4]=FLAG;
        int res=0; 
        res=write(fd,DISCBUF,strlen(DISCBUF));
        sleep(1);
        
        while(state!=5){
            read(fd,buf,1);
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
        }
    }

    printf("DISCONNECTING\n");

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    close(fd);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llread(char* packet)
{
    int readstate=0;
    int bytes_read=0;
    unsigned char readbuffer[Max_Size];
    
    unsigned char messagebuffer[Max_Size];

    unsigned char message[Max_Size];
    unsigned char bcc2 = 0x00;


    unsigned char C0 = CONTROLFIELD_S(0);              
    unsigned char C1 = CONTROLFIELD_S(1);


    unsigned char controlbuffer[Max_Size];

    while(readstate!=6 && readstate!=7){
        
        read(fd,readbuffer,1);
        switch(readstate){
            case 0://start
                if(readbuffer[0]==FLAG){
                    readstate=1;
                }
                else{
                    readstate=0;
                }   
                break;
            case 1://flag
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
            case 2://A
                if(readbuffer[0] == C0 || readbuffer[0] == C1)
                {
                    readstate = 3;
                    controlbuffer[0]  = readbuffer[0];
                }
                else if(readbuffer[0]==FLAG){
                    readstate=1;
                }
                else{
                    readstate=0;
                } 
                break;
            case 3:///C
                if(readbuffer[0]==A_Tx ^ controlbuffer[0]){
                    readstate=4;
                }
                else if(readbuffer[0]==FLAG){
                    readstate=1;
                }
                else{
                    readstate=0;
                }  
                break;
            case 4:///BCC1  e receber dados
                bcc2=0x00;
                for(int i=0; i<n ;i++){                                                       ///esta a dar erro so da a ultima character
                    bcc2=bcc2^message[i];
                }
                if(readbuffer[0]==bcc2){
                    readstate=5;
                }
                else{
                    message[n]=readbuffer[0];
                    n++;
                }
                break;
            case 5://BCC2
                if(readbuffer[0]==FLAG){
                    readstate=6;
                }
                
                else{
                    readstate = 7;//erro
                }
                break;
            default:
                break;
        }

     
    }

    unsigned char output[strlen(message)*2];
    int out_len=0;
    for (int j = 0; j < strlen(message); j++) {
        if (message[j] == ESCAPE_OCTET) {
            if (j + 1 < strlen(message) && (message[j + 1] == (0x7C ^ XOR_OCTET))) {
                out[out_len++] = FLAG;
                j++; 
            } else if (j + 1 < strlen(message) && (message[j + 1] == (0x7D ^ XOR_OCTET))) {
                out[out_len++] = ESCAPE_OCTET;
                j++;  
            } else {
                out[out_len++] = message[j];
            }
        } else {
            out[out_len++] = message[j];
        }
    }
    strcpy(packet, output);




    messagebuffer[0] = FLAG;
    messagebuffer[1] = A_Rx;
               
    if(controlbuffer[0]==0x80){         ///C0     ////RR ou REJ(0 ou 1 se controlbuffer[0] for 0 ou 1)
        if(readstate==7){
            messagebuffer[2] = REJ(1); 
        }
        else{
            messagebuffer[2] = RR(1); 
        }
    }
    else if(controlbuffer[0]==0xC0){     ///C1
        if(readstate==7){
            messagebuffer[2] = REJ(0); 
        }
        else{
            messagebuffer[2] = RR(0); 
        }
    }
    messagebuffer[3] = messagebuffer[1] ^ messagebuffer[2];
    messagebuffer[4] = FLAG;
       
    write(fd,messagebuffer,strlen(messagebuffer));
    sleep(1);

    return out_len;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int llwrite(char* buf, int bufSize)
{
   int frame_to_send = 0,state=0;
    unsigned char bcc2;
    unsigned char rcvbuf[Max_Size],controlframe[1];


    unsigned char outbuf[bufSize*2];
    int out_len=0;
    outbuf[out_len++]=FLAG;
    outbuf[out_len++]=A_Rx;
    outbuf[out_len++]=CONTROLFIELD_S(frame_to_send);
    outbuf[out_len++]=A_Rx ^ CONTROLFIELD_S(frame_to_send);

    for (size_t i = 0; i < bufsize; i++) {/////////stuffing
        bcc2 ^= buf[i];
        if (buf[i] == FLAG) {
            outbuf[out_len++] = ESCAPE_OCTET;
            outbuf[out_len++] = 0x7C ^ XOR_OCTET;
        } else if (buf[i] == ESCAPE_OCTET) {
            outbuf[out_len++] = ESCAPE_OCTET;
            outbuf[out_len++] = 0x7D ^ XOR_OCTET;
        } else {
            outbuf[out_len++] = buf[i];
        }
    }
    outbuf[out_len++]=bcc2;
    outbuf[out_len++]=FLAG;

    write(fd,buf,out_len);
    sleep(1);

    unsigned char RR = RR(!frame_to_send);
    unsigned char REJ = REJ(frame_to_send);
    frame_to_send = !frame_to_send;

    while (state != 5){
        read(fd,rcvbuf,1);
        switch (state){
            case 0:
                if(rcvbuf[0] == FLAG){
                    state = 1;                        
                }
                break;
            case 1:
                if(rcvbuf[0] == A_Rx){
                    state = 2;
                }
                else if(rcvbuf[0] != FLAG){
                    state = 0;
                }
                break;
            case 2:
                if(rcvbuf[0] == FLAG){
                    state = 1;
                }
                else if(rcvbuf[0] == REJ || rcvbuf[0] == RR){
                    state = 3;
                    controlframe[0] = rcvbuf[0];    
                }
                else{
                    state = 0;
                }
                break;
            case 3:
                if(rcvbuf[0] == (A_Rx ^ controlframe[0])){
                    state = 4;   
                }
                else if(rcvbuf[0] == FLAG){
                    state = 1;
                }
                else{
                    state = 0;
                }
                break;
            case 4:
                if(rcvbuf[0] == FLAG){
                    state = 5;
                }
                else{
                    state = 0;
                }
                break;
            default:
                break;
        }
    }

    /////////faltava a cena das retransmissions
    
    return 1;
}
