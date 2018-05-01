
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <boost/crc.hpp>
#include <mutex>


//methods.h
#include "methods.h"
#include "socketMethods.h"

#define WINDOWSIZE 3

using namespace std;

mutex mtx;


vector<string> buffArr;
char strCopy[PACKETSIZE] = {0};
int finished = 0;
const int packetSize = PACKETSIZE;
const int buffSize = packetSize - 2;
char buff[buffSize] = {0};
char server_reply[1] = {0};
char packet[packetSize] = {0};
char genPacket[packetSize] = {0};
char chksum[8] = {0};
int readSize = buffSize;
int dropped = 0;
int seqNum = 0;   
int total = 0;
int inc = 0;
int hSocket = 0;
int WindowToSeqNum = 0;
bool run = true;
int reads = 0;
clock_t t1, t2;
double timeout = 50.0;

///////////////////////////





void printWindow(int seqNum, int i) {
    int windowStart = seqNum-WINDOWSIZE;
    int unit = (seqNum-(WINDOWSIZE-i));
    int end = seqNum-1;

    if (windowStart < 0) {
        windowStart = windowStart+SEQUENCENUM+1;
        unit = unit+SEQUENCENUM+1;
        end = end+SEQUENCENUM+1;
    }

    for (int k = 0; k <= 9; k++) {

        if(k == windowStart)
            cout << "[";

        if(k == unit)
            cout << "(";

        cout << k;

        if(k == unit)
            cout << ")";

        if(k == end)
            cout << "]";

        cout << " ";
    }
    cout << endl;
}



void SlidingWindowSend() {

    while (run == true || buffArr.size() > 0) {

        mtx.lock();
        for(int i = 0; i < buffArr.size(); i++) { //sending

            cout << "sending packet " << total+i;
            // printWindow(*seqNum, i);
            cout << " [" << buffArr.at(i)[0] << "]\tsize: " << buffArr.size() << endl;

            memset(packet, 0, packetSize);
            strcpy(packet, buffArr.at(i).c_str());
            // cout << "packetsize: " << sizeof packet << "\tpacket: " << packet << endl;
            if(shouldFail() == true) {
                cout << "Faked Error: Send Got Lost" << endl;
                dropped++;
            }
            else {
                while(SocketSend(hSocket, packet, packetSize) == -1) {
                    cout << "resending due to packet drop" << endl;
                }
            }
        }
        mtx.unlock();

        clock_t time = clock();

        while (diffclock(clock(), time) <= timeout) {
        }

    }
    // cout << endl << endl;
}

int SlidingWindowRecvSR(double timeout) {

    cout << "Receive Start" << endl;

    while(run == true || buffArr.size() > 0) {

        while(SocketReceive(hSocket, server_reply, sizeof(server_reply)) > 0) {
            mtx.lock();

            cout << "Ack Received: " << server_reply[0] << endl;

            if (server_reply[0] == buffArr.at(0)[0]) {
                buffArr.erase(buffArr.begin());
                total++;
            } else {
                (dropped)++;
            }
            mtx.unlock();
        }
    }
    return 0;
}

int SlidingWindowRecvGBN(double timeout) {

    cout << "Receive Start" << endl;

    while(run == true || buffArr.size() > 0) {

        while(SocketReceive(hSocket, server_reply, sizeof(server_reply)) > 0) {
            mtx.lock();

            cout << "Ack Received: " << server_reply[0] << endl;

            if (server_reply[0] == buffArr.at(inc)[0]) {

                inc++;

                if(inc == buffArr.size()) {
                    buffArr.clear();
                    total += WINDOWSIZE;
                    inc = 0;
                }

            } else {
                (dropped)++;
            }
            mtx.unlock();
        }
    }
    return 0;
}

void loadBuffer(ifstream *myfile) {
    while (myfile->read(buff, buffSize))
    {
        reads++;
        do {
            if(buffArr.size() == WINDOWSIZE) {

            } else {
                mtx.lock();
                readSize = getReadSize(buff, readSize);

                // char buffCopy[buffSize];
                // strcpy(buffCopy, buff);
                // checksum(buffCopy, chksum);
                // generateChecksumPacket(genPacket, buffCopy, chksum, readSize, seqNum);
                generatePacket(genPacket, buff, readSize, seqNum);
                buffArr.push_back(genPacket);

                seqNum++;
                    if(seqNum > SEQUENCENUM)
                        seqNum = 0;
                readSize = buffSize;
                memset(buff, 0, buffSize);
                memset(genPacket, 0, packetSize);
                memset(chksum, 0, sizeof(char)*8);
                mtx.unlock();
            }
        } while(buffArr.size() == WINDOWSIZE);
    }

    mtx.lock();
    reads++;
    readSize = getReadSize(buff, readSize);
    // char buffCopy[buffSize];
    // strcpy(buffCopy, buff);
    // checksum(buffCopy, chksum);
    // generateChecksumPacket(genPacket, buffCopy, chksum, readSize, seqNum);
    generatePacket(genPacket, buff, readSize, seqNum);
    buffArr.push_back(genPacket);

    seqNum++;
        if(seqNum > SEQUENCENUM)
            seqNum = 0;
    readSize = buffSize;
    memset(buff, 0, buffSize);
    memset(genPacket, 0, packetSize);

    mtx.unlock();
    run = false;
}



int SlidingWindowProtocol(int argc, char *argv[], int type, int timeout) {
    if (argc != 2)
    {
        printf("Missing File Argument\n");
        return 0;
    }
    /// Variable declaration ///    
    streampos begin, end;
    string line;
    
    t1 = clock();
    ifstream myfile(argv[1], ios::binary);
    if (!myfile.is_open())
    {
        return -1;
    }

    hSocket = clientStart();

    thread reader(loadBuffer, &myfile);
    thread sender(SlidingWindowSend);
    if(type == 0){
        thread receiver(SlidingWindowRecvGBN, (double) timeout);
        reader.join();
        sender.join();
        receiver.join();
    } else {
        thread receiver(SlidingWindowRecvSR, (double) timeout);
        reader.join();
        sender.join();
        receiver.join();
    }

    


    myfile.close();

    t2 = clock();
    float diff ((float)t2-(float)t1);
    float seconds = diff / CLOCKS_PER_SEC;    
    

    cout << "\n\n\n" << endl;
    cout << "total sends dropped: " << dropped << endl;
    cout << "number of packets: " << total << endl;
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