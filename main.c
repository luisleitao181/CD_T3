#include "linklayer.h"
#include "linklayer.c"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
// #define BAUDRATE B38400
// #define MODEMDEVICE "/dev/ttyS1"
// #define _POSIX_SOURCE 1 /* POSIX compliant source */
// #define FALSE 0
// #define TRUE 1
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("usage: progname /dev/ttySxx tx|rx filename\n");
        exit(1);
    }

    printf("%s %s %s\n", argv[1], argv[2], argv[3]);
    fflush(stdout);

    if (strcmp(argv[2], "tx") == 0)
    {
        // ***********
        // tx mode
        printf("tx mode\n");

        // open connection
        struct linkLayer ll;
        sprintf(ll.serialPort, "%s", argv[1]);
        ll.role = TRANSMITTER;
        ll.baudRate = 9600;
        ll.numTries = 3;
        ll.timeOut = 3;

        if(llopen(ll)==-1) {
            fprintf(stderr, "Could not initialize link layer connection\n");
            exit(1);
        }

        printf("connection opened\n");
        fflush(stdout);
        fflush(stderr);
//////////////////////////////////////
        // open file to read
        char *file_path = argv[3];
        int file_desc = open(file_path, O_RDONLY);
        if(file_desc < 0) {
            fprintf(stderr, "Error opening file: %s\n", file_path);
            exit(1);
        }

        // cycle through
        const int buf_size = MAX_PAYLOAD_SIZE-1;
        unsigned char buffer[buf_size+1];
        int write_result = 0;
        int bytes_read = 1;
        while (bytes_read > 0)
        {
            bytes_read = read(file_desc, buffer+1, buf_size);
            if(bytes_read < 0) {
                    fprintf(stderr, "Error receiving from link layer\n");
                    break;
            }
            else if (bytes_read > 0) {
                // continue sending data
                buffer[0] = 1;
                write_result = llwrite(buffer, bytes_read+1);
                if(write_result < 0) {
                    fprintf(stderr, "Error sending data to link layer\n");
                    break;
                }
                printf("read from file -> write to link layer, %d\n", bytes_read);
            }
            else if (bytes_read == 0) {
                // stop receiver
                buffer[0] = 0;
                llwrite(buffer, 1);
                printf("App layer: done reading and sending file\n");
                break;
            }

            sleep(1);
        }
        // close connection
        llclose(ll,1);
        close(file_desc);
        return 0;
    }
/////////////////////////////////////////////////




    else
    {
        // ***************
        // rx mode
        printf("rx mode\n");

        struct linkLayer ll;
        sprintf(ll.serialPort, "%s", argv[1]);
        ll.role = RECEIVER;
        ll.baudRate = 9600;
        ll.numTries = 3;
        ll.timeOut = 3;

        if(llopen(ll)==-1) {
            fprintf(stderr, "Could not initialize link layer connection\n");
            exit(1);
        }

        char *file_path = argv[3];
        int file_desc = open(file_path, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if(file_desc < 0) {
            fprintf(stderr, "Error opening file: %s\n", file_path);
            exit(1);
        }

        int bytes_read = 0;
        int write_result = 0;
        const int buf_size = MAX_PAYLOAD_SIZE;
        unsigned char buffer[buf_size];
        int total_bytes = 0;

        while (bytes_read >= 0)
        {
            bytes_read = llread(buffer);
            if(bytes_read < 0) {
                fprintf(stderr, "Error receiving from link layer\n");
                break;
            }
            else if (bytes_read > 0) {
                if (buffer[0] == 1) {
                    write_result = write(file_desc, buffer+1, bytes_read-1);
                    if(write_result < 0) {
                        fprintf(stderr, "Error writing to file\n");
                        break;
                    }
                    total_bytes = total_bytes + write_result;
                    printf("read from file -> write to link layer, %d %d %d\n", bytes_read, write_result, total_bytes);
                }
                else if (buffer[0] == 0) {
                    printf("App layer: done receiving file\n");
                    break;
                }
            }
        }

        llclose(ll,1);
        close(file_desc);
        return 0;
    }
}







// volatile int STOP=FALSE;

// int main(int argc, char** argv)
// {
//     int fd,c, res,state=0,saiwhile=0;
//     struct termios oldtio,newtio;
//     unsigned char buf[7],rcvbuf[10];
//     int i, sum = 0, speed = 0;

//     if ( (argc < 2) ||
//          ((strcmp("/dev/ttyS0", argv[1])!=0) &&
//           (strcmp("/dev/ttyS1", argv[1])!=0) )) {
//         printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
//         exit(1);
//     }

//     const char *serialPort = argv[1];
//     const char *role = argv[2];
//     const char *filename = argv[3];


    
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////
//     Role role;

//   if(!strcmp("t",role))
//   {
//     role = transmitter;                    //nao sei se isto e assim
//     //printf("Role: TRANSMITER\n");
//   }
//   else if(!strcmp("r",role))
//   {
//     role = reciever;
//     //printf("Role: RECEIVER\n");
//   }
//   LinkLayer params;
//   params.role = role;
//   strcpy(params.serialPort,serialPort);

//   int fdsize=llopen(params)
//   if(fd<0)exit(-1);



// ///////////////////////////////////////////////////////////////////////////////////////////////////////////

//   if(role == transmitter)
//   {
//   }
//   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   else if(role == reciever)
//   {
//   }

//   return 0;
// }
