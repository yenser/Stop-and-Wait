
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/crc.hpp>


//methods.h
#include "methods.h"
#include "socketMethods.h"

#define WINDOWSIZE 3

using namespace std;


vector<string> buffArr;
char strCopy[PACKETSIZE] = {0};
///////////////////////////









void SlidingWindowSend(int hSocket, char* packet, int packetSize, int* total, int* dropped, int* seqNum) {

    for(int i = 0; i < buffArr.size(); i++) { //sending

        memset(packet, 0, packetSize);
        cout << "sending packet " << *total << " SeqNum: " << i << endl;
        strcpy(packet, buffArr.at(i).c_str());

        while(SocketSend(hSocket, packet, packetSize) == -1) {
            cout << "resending due to packet drop" << endl;
            (*dropped)++;
        }

        (*total)++; 
    }

    cout << endl;
}

void SlidingWindowRecv(int hSocket, char* server_reply, int* total, int* dropped, int* seqNum, int* inc) {

    double timeout = 0.5;
    clock_t time = clock();


    while(SocketReceive(hSocket, server_reply, sizeof(server_reply)) == -1) {
        if (diffclock(clock(), time) >= timeout) {
            
            // SlidingWindowSend(hSocket, packet, packetSize, &total, &dropped, &seqNum);

            (*dropped)++;
            cout << "resending previous data " << *total << endl;
            timeout += 0.5;
        } else {
            cout << "waiting for timeout... " << diffclock(clock(), time) << " | " << timeout << endl;
        }
    }
    cout << "Received Ack " << server_reply[0] << endl << endl;
    if (server_reply[0] == buffArr.at(0)[0]) {
        buffArr.erase(buffArr.begin());
        (*inc)--;
    }

}


int SlidingWindowProtocol(int argc, char *argv[]) {
    if (argc != 2)
    {
        printf("Missing File Argument\n");
        return 0;
    }

    /// Variable declaration ///    
    streampos begin, end;
    string line;
    const int packetSize = PACKETSIZE;
    const int buffSize = packetSize - 2;
    char buff[buffSize] = {0};
    char server_reply[1] = {0};
    char packet[packetSize] = {0};
    int readSize = buffSize;
    int dropped = 0;
    int seqNum = 0;   
    int total = 0;
    int inc = 0;
    int WindowToSeqNum = 0;
    clock_t t1, t2;
    t1 = clock();


    ifstream myfile(argv[1], ios::binary);
    if (!myfile.is_open())
    {
        return -1;
    }
    int hSocket = clientStart();




    while (myfile.read(buff, buffSize))
    {
        

        readSize = getReadSize(buff, readSize);
        generatePacket(packet, buff, readSize, seqNum);
        buffArr.push_back(packet);

        if(inc == WINDOWSIZE) {
            // cout << "\n---New Seqence---\n" << endl;
            

            SlidingWindowSend(hSocket, packet, packetSize, &total, &dropped, &seqNum);



            

            inc = 0;
        } else {
            inc++;  
        }

        seqNum++;
            if(seqNum > SEQUENCENUM)
                seqNum = 0;
        readSize = buffSize;
        memset(buff, 0, buffSize);
        memset(packet, 0, packetSize);
    }


    // Stuff for final read
    readSize = getReadSize(buff, readSize);
    if (readSize != 0) {
        generatePacket(packet, buff, readSize, inc);
        buffArr.push_back(packet);
        inc++;
    }

    SlidingWindowSend(hSocket, packet, packetSize, &total, &dropped, &seqNum);




    myfile.close();

    t2 = clock();
    float diff ((float)t2-(float)t1);
    float seconds = diff / CLOCKS_PER_SEC;    
    

    cout << "\n\n\n" << endl;
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