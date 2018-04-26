#ifndef _METHODS
#define _METHODS

using namespace std;

#define PORT 9191
#define PACKETSIZE 1024
#define SEQUENCENUM 9
#define thing0 "127.0.0.1"
// #define thing0 "10.35.195.46"

typedef unsigned short int u_short; 
typedef unsigned long int u_long;

typedef struct Packet {
	char seqNum;
    char body[PACKETSIZE-2] = {0};
} pktype;



string exec(const char*);

ifstream::pos_type filesize(const char*);

int getReadSize(char*, int);

void catPacket(char*, char*, int);
void catPacketStruct(char*, char*, int);

void generatePacket( char*, char*, int, int);

void serverCutAndWrite();

u_short cksum(u_short*, int);

bool shouldFail();

double diffclock(clock_t ,clock_t); 

#endif 