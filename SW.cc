
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


char buffArray[SEQUENCENUM+1][PACKETSIZE-2] = {};
///////////////////////////



int get2DArraySize() {
    return sizeof buffArray / sizeof buffArray[0];
}








void SlidingWindow(int hSocket, char* packet, int packetSize, char* buff, char* server_reply, int buffSize, int* readSize, int* total, int* dropped, int* seqNum) {
    double timeout = 0.5;
    clock_t time = clock();


    // checksum stuff
    // boost::crc_32_type result;
    // result.process_bytes(buff, buffSize);
    // cout << "CheckSum Value: " << std::hex << std::uppercase << result.checksum() << endl;
    // ^ checksum stuff

    for(int i = 0; i <= SEQUENCENUM; i++) {

        cout << "sending packet " << total << " SeqNum: " << i << endl;

        generatePacket(packet, buffArray[i], *readSize, i);

        while(SocketSend(hSocket, packet, packetSize) == -1) {
            cout << "resending due to packet drop" << endl;
            (dropped)++;
        }

        (total)++;

    }

    cout << endl;



    // // if(shouldFail() == true) {
    // //     cout << "Fake Error: Socket did not send" << endl;
    // // } else {
    //     while(SocketSend(hSocket, packet, packetSize) == -1) {
    //         cout << "resending due to packet drop" << endl;
    //         (dropped)++;
    //     }
    // // }
    
    // //wait for reply
    // while(SocketReceive(hSocket, server_reply, sizeof(server_reply)) == -1) {
    //     if (diffclock(clock(), time) >= timeout) {
    //         SocketSend(hSocket, packet, packetSize);
    //         (dropped)++;
    //         cout << "resending previous data " << total << endl;
    //         timeout += 0.5;
    //     } else {
    //         cout << "waiting for timeout... " << diffclock(clock(), time)<< " | " << timeout << endl;
    //     }
    // }
    // cout << "Received Ack " << server_reply[0] << endl << endl;

    
    //variable resets
    // memset(buff, 0, sizeof(char)*buffSize);   
    // memset(packet, 0, sizeof(char)*packetSize);
    // seqNum = (seqNum+1) % 2;
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


    ifstream myfile(argv[1], ios::binary);


    if (!myfile.is_open())
    {
        return -1;
    }

    int hSocket = clientStart();

    clock_t t1, t2;
    t1 = clock();

    int inc = 0;

    while (myfile.read(buff, buffSize))
    {
        if(inc == SEQUENCENUM) {
            // cout << "\n---New Seqence---\n" << endl;
            SlidingWindow(hSocket, packet, packetSize, buff, server_reply, buffSize, &readSize, &total, &dropped, &seqNum);
            inc = 0;
        } else {
            memset(buffArray[inc], 0, sizeof(buffArray[inc]));
            strcpy(buffArray[inc], buff);
            // cout << buffArray[inc];

            inc++;
        }
    }
    readSize = getReadSize(buff, readSize);
    if (readSize != 0) {
        if(inc == SEQUENCENUM) {
            SlidingWindow(hSocket, packet, packetSize, buff, server_reply, buffSize, &readSize, &total, &dropped, &seqNum);
            inc = 0;
        } else {
            inc++;
        }
    }


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