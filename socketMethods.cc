#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "methods.h"

using namespace std;

short SocketCreate(void)
{
	short hSocket;
	printf("Create the socket\n");
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}
int BindCreatedSocket(int hSocket)
{
	int iRetval = -1;
	int ClientPort = PORT;
	struct sockaddr_in remote = {0};
	remote.sin_family = AF_INET;				/* Internet address family */
	remote.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	remote.sin_port = htons(ClientPort);		/* Local port */
	iRetval = ::bind(hSocket, (struct sockaddr *)&remote, sizeof(remote));
	return iRetval;
}

//try to connect with server
int SocketConnect(int hSocket)
{
    int iRetval = -1;
    int ServerPort = PORT;
    struct sockaddr_in remote = {0};
    remote.sin_addr.s_addr = inet_addr(thing0); //Local Host
    remote.sin_family = AF_INET;
    remote.sin_port = htons(ServerPort);
    iRetval = connect(hSocket, (struct sockaddr *)&remote, sizeof(struct sockaddr_in));
    return iRetval;
}

// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int hSocket, char *Rqst, short lenRqst)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 0; /* 20 Secs Timeout */
    tv.tv_usec = 1000;
    if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    return shortRetval;
}

//receive the data from the server
int SocketReceive(int hSocket, char *Rsp, short RvcSize)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 0; /* 20 Secs Timeout */
    tv.tv_usec = 1000;
    if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    // printf("Response %s\n", Rsp);
    return shortRetval;
}

int serverStart() {
    int socket_desc, sock, clientLen;
	struct sockaddr_in client;
	//Create socket
	socket_desc = SocketCreate();
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		return 1;
	}
	printf("Socket created\n");
	//Bind
	if (BindCreatedSocket(socket_desc) < 0)
	{
		//print the error message
		perror("bind failed.");
		return 1;
	}
	printf("bind done\n");
	//Listen
	listen(socket_desc, 3);
	//Accept and incoming connection
	printf("Waiting for incoming connections...\n");
	clientLen = sizeof(struct sockaddr_in);
	//accept connection from an incoming client
	sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&clientLen);
	if (sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	printf("Connection accepted\n");

    return sock;
}

int clientStart() {
    int hSocket;
    //Create socket
    hSocket = SocketCreate();
    if (hSocket == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket is created\n");
    //Connect to remote server
    if (SocketConnect(hSocket) < 0)
    {
        perror("connect failed.\n");
        return 1;
    }
    printf("Sucessfully conected with server\n");

    return hSocket;
}

bool sendACK(int sock, char* response) {
    cout << "Sending ack " << response[0] << "... ";
    // printf("Sending ack... %d\n", ackNum);

    if(shouldFail() == true) {
        cout << "Fake Error: Ack got lost" << endl;
        return true;
    } else {
        send(sock, response, sizeof(response), 0);
        cout << endl;	
        return false;	
    }
}


