#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <boost/crc.hpp>
#include "methods.h"


//md5sum libraries
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

using namespace std;


string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

ifstream::pos_type filesize(const char* filename)
{
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

void checksum(char* buff, char* chksum)
{
  std::hash<std::string> string_hash;
  int h = string_hash(buff);
  stringstream ss;
  ss << hex << h;
  std::string hString = ss.str();
  strcpy(chksum, hString.c_str());
}

int getReadSize(char* buff, int size) {
    int i = size;
    while(buff[i-1] == '\0') {
        i--;
    }
    return i;
}

void catPacket(char* packet, char* buff, int readSize) {
        for(int i = 0; i < readSize; i++) {
            packet[1+i] = buff[i];
        }
}

void catPacketStruct (char* body, char* buff , int readSize) {
    for (int i = 0; i < readSize; i++)
    {
        body[i] = buff[i];
    }
}

void generatePacket( char* packet, char* buff, int readSize, int seqNum) {
	//GENERATE PACKET TEMPLATE///
    packet[0] = '0' + seqNum;
    catPacket(packet, buff, readSize);
    packet[readSize+1] = '0' + seqNum;
}

void generateChecksumPacket( char* packet, char* buff, char* chksum, int readSize, int seqNum) {
	//GENERATE PACKET TEMPLATE///
    packet[0] = '0' + seqNum;
    for(int i = 0; i < readSize; i++) {
        packet[1+i] = buff[i];
    }
    for(int j = readSize; j < readSize+8; j++) {
        packet[1+j] = chksum[j-readSize];
    }
    packet[readSize+9] = '0' + seqNum;
}

bool shouldFail() {
    long nonce = 100;
    const long A = 48271;
    const long M = 2147483647;
    const long Q = M/A;
    const long R = M%A;
	static long state = 1;
	long t = A * (state % Q) - R * (state / Q);
	if (t > 0)
		state = t;
	else
		state = t + M;
	return (long)(((double) state/M)* nonce) == 1;
}

u_short cksum(u_short *buf, int count)
{
    u_long sum = 0;
    while (count--)
    {
        sum += *buf++;
        if(sum & 0xFFFF0000)
        {
            /* carry occurred, so wrap around */
            sum &= 0xFFF;
            sum++;
        }
    }
    return ~ (sum & 0xFFFF);
}

double diffclock(clock_t clock1,clock_t clock2)
{
    // double diffticks= 1000.0* (clock1-clock2);
    double diffms=1000.0 * (clock1 - clock2) / CLOCKS_PER_SEC;
    return diffms;
}
