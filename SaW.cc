
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <boost/crc.hpp>


//methods.h
#include "methods.h"
#include "socketMethods.h"

using namespace std;


int checksumFails = 0;
int didChecksumFail = false;

void stopAndWait(int hSocket, char* packet, int packetSize, char* buff, char* chksum, char* server_reply, int buffSize, int* readSize, int* total, int* dropped, int* seqNum, double timeoutVal) {

    double timeout = timeoutVal;
    clock_t time = clock();

    char buffCopy[buffSize];
    strcpy(buffCopy, buff);

    checksum(buffCopy, chksum);

    cout << "sending packet " << *total << endl;

    generateChecksumPacket(packet, buffCopy, chksum, *readSize, *seqNum); //assemble packet

    //cout << "This is the packet: " << packet << endl;
    didChecksumFail = shouldFail();
    if(didChecksumFail) {
        checksumFails++;
        cout << "Fake Error: Checksum" << endl;
        packet[3] = packet[3]+15;
    }

    if(shouldFail() == true) {
        cout << "Fake Error: Socket did not send" << endl;
    } else {
        while(SocketSend(hSocket, packet, packetSize) == -1) {
            cout << "resending due to packet drop" << endl;
            (*dropped)++;
        }
    }

    if(didChecksumFail) {
        packet[3] = packet[3]-15;
    }

    //wait for reply
    while(SocketReceive(hSocket, server_reply, sizeof(server_reply)) == -1) {
        if (diffclock(clock(), time) >= timeout) {
            SocketSend(hSocket, packet, packetSize);
            (*dropped)++;
            cout << "resending previous data " << *total << endl;
            timeout += 0.5;
        } else {
            cout << "waiting for timeout... " << diffclock(clock(), time)<< " | " << timeout << endl;
        }
    }
    cout << "Received Ack " << server_reply[0] << endl << endl;


    //variable resets
    memset(buff, 0, sizeof(char)*buffSize);
    memset(packet, 0, sizeof(char)*packetSize);
    memset(chksum, 0, sizeof(char)*8);
    *seqNum = (*seqNum+1) % 2;
    (*total)++;

}

int stopAndWaitProtocol(int argc, char *argv[], int timeout) {
    if (argc != 2)
    {
        printf("Missing File Argument\n");
        return 0;
    }

    /// Variable declaration ///
    streampos begin, end;
    string line;
    ifstream myfile(argv[1], ios::binary);
    const int packetSize = PACKETSIZE;
    //cout << "Packetsize: " << packetSize << endl;
    const int buffSize = packetSize - 10;
    //cout << "Buffsize: " << buffSize << endl;
    char buff[buffSize+1] = {0};
    char chksum[8] = {0};
    char server_reply[1] = {0};
    char packet[packetSize] = {0};
    int readSize = buffSize;
    int dropped = 0;
    int seqNum = 0;
    int total = 0;
    ///////////////////////////


    if (!myfile.is_open())
    {
        return -1;
    }

    int hSocket = clientStart();

    clock_t t1, t2;
    t1 = clock();

    //Send data to the server
    while (myfile.read(buff, buffSize))
    {
        stopAndWait(hSocket, packet, packetSize, buff, chksum, server_reply, buffSize, &readSize, &total, &dropped, &seqNum, (double) timeout);
    }
    readSize = getReadSize(buff, readSize);
    if (readSize != 0) {
        stopAndWait(hSocket, packet, packetSize, buff, chksum, server_reply, buffSize, &readSize, &total, &dropped, &seqNum, (double) timeout);
    }


    myfile.close();

    t2 = clock();
    float diff ((float)t2-(float)t1);
    float seconds = diff / CLOCKS_PER_SEC;


    cout << "\n\n\n" << endl;
    cout << "total checksums failed: " << checksumFails << endl;
    cout << "total sends dropped: " << dropped << endl;
    cout << "Number of Packets: " << total << endl;
    cout << "file size: " << filesize(argv[1]) << " bytes" << endl;
    cout << "total elapsed time: " << seconds << endl;
    string filename (argv[1]);
    string fileSum = "md5 " + filename;
    // string fileSum = "md5sum " + filename;
    cout << "MD5SUM: " << exec(fileSum.c_str()) << endl;

    close(hSocket);
    shutdown(hSocket, 0);
    shutdown(hSocket, 1);
    shutdown(hSocket, 2);
    return 0;
}
