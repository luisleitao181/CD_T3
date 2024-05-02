#ifndef _functions_H_ 
#define _functions_H_ 
typedef enum{
    reciever, 
    transmitter,
} Role;
typedef struct{
    char SerialPort[100];
    Role r;
    int baudrate;
} linklayer;


int llopen(linklayer parameters);
int llwrite(linklayer parameters);
int llread(linklayer parameters);
int llclose(linklayer parameters);

#endif
