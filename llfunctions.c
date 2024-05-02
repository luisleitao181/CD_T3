/*FUNCTIONS*/

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

int llopen(int ID)
{
    if(ID==0){
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
    if(ID==1){
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
