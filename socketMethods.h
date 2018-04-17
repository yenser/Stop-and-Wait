#ifndef _SOCKETMETHODS
#define _SOCKETMETHODS

using namespace std;

short SocketCreate(void);

int BindCreatedSocket(int);

int SocketConnect(int);

int SocketSend(int, char*, short);

int SocketReceive(int, char *, short);

int serverStart();

int clientStart();

bool sendACK(int, char*, int);



    

#endif 