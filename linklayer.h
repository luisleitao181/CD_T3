typedef struct linkLayer{
    char serialPort[50];
    int role; //defines the role of the program: 0==Transmitter, 1=Receiver
    int baudRate;
    int numTries;
    int timeOut;
    int nRetransmissions;
} linkLayer;

//ROLE
#define NOT_DEFINED -1
#define TRANSMITTER 0
#define RECEIVER 1
#define MODEMDEVICE "/dev/ttyS1"
#define Max_Size 250
//SIZE of maximum acceptable payload; maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

//
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
//



//CONNECTION deafault values
#define BAUDRATE_DEFAULT B38400
#define MAX_RETRANSMISSIONS_DEFAULT 3
#define TIMEOUT_DEFAULT 4
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//MISC
#define FALSE 0
#define TRUE 1

// Opens a connection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters);
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize);
// Receive data in packet
int llread(char* packet);
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(linkLayer connectionParameters, int showStatistics);

#endif
