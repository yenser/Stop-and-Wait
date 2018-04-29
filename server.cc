#include <stdio.h>
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

using namespace std;

/// Variable declaration ///
const int packetSize = PACKETSIZE;
const int buffSize = packetSize-2;
char client_message[packetSize+1] = {0};
// memset(client_message, '\0', sizeof client_message);
char buff[buffSize]={0};
char chksum[9] = {0};
char chksumCompare[9] = {0};
char response[1] = {'0'};
int ackNum = 0;
int total = 0;
int sock = serverStart();
///////////////////////////

int getEndIndex(char* buff, int size) {
	int i = size;
	while(buff[i] == '\0'){
		i--;
	}
	return i;
}

void stopAndWait() {

	//setup file
	ofstream myfile;
	myfile.open("mylogServer.txt", ios::binary);


	clock_t t1, t2;
	int failed = 0;
	int writes = 0;
	t1 = clock();

	while(recv(sock, client_message, packetSize, 0) > 0) {
		cout << endl << "Received " << total << "(" << client_message[0] << ")" << endl;

		bool shouldFail = false;

		if ((ackNum+48) == client_message[0]) {

			int end = getEndIndex(client_message, packetSize);
			memset(buff, 0, sizeof buff);
			if (end != 0) {
				// cout << "writing " << total-1 << endl;
				writes++;
				copy(client_message + 1, client_message + end-8, buff);
				copy(client_message + end-8, client_message + end, chksum);

		    checksum(buff, chksumCompare);
		    if(strcmp(chksum, chksumCompare) == 0) {
					myfile.write((char*) &buff, end-9);
			  }
				else {
					cout << "Checksum did not match" << endl;
				}

			}

			memset(client_message, '\0', sizeof client_message); // clear client_message
			memset(chksum, '\0', sizeof chksum);
			memset(chksumCompare, '\0', sizeof chksumCompare);


			response[0] = '0'+ackNum;
			ackNum = (ackNum+1) % 2;
		}

		shouldFail = sendACK(sock, response);

		if (shouldFail == false) {
			total++;
		} else {
			failed++;
		}
	}


	myfile.close();
	close(sock);

	t2 = clock();
	float diff ((float)t2-(float)t1);
	float seconds = diff / CLOCKS_PER_SEC;


    cout << "\n\n\n" << endl;
	cout << "total ACKs dropped: " << failed << endl;
	// cout << "writes to file: " << writes << endl;
    cout << "Number of Packets: " << total << endl;
    cout << "file size: " << filesize("mylogServer.txt") << " bytes" << endl;
    cout << "total elapsed time: " << seconds << endl;
	cout << "MD5SUM: " << exec("md5 mylogServer.txt") << endl;
	// cout << "MD5SUM: " << exec("md5sum mylogServer.txt") << endl;
}


// char buffArray[SEQUENCENUM+1][PACKETSIZE+1] = {};
vector<string> buffArr;
char strCopy[packetSize] = {0};



void slidingWindow() {
		//setup file
	ofstream myfile;
	myfile.open("mylogServer.txt", ios::binary);

	if (!myfile.is_open())
    {
    	cout << "file isn't open" << endl;
    }

	clock_t t1, t2;
	int failed = 0;
	int writes = 0;
	t1 = clock();

	int i = 0;

	while(recv(sock, client_message, packetSize, 0) > 0) {

		cout << "recieved " << total << " [" << client_message[0] << "]\texpecting " << i << endl;


		if ((i+48) == client_message[0]) {
		// if(i == SEQUENCENUM) { // first reply if needs more info, then write everything
			total++;

			memset(buff, 0, sizeof buff);

			int end = getEndIndex(client_message, packetSize);

			if (end != 0) {
				writes++;
				copy(client_message+1, client_message + end, buff);
				myfile.write((char*) &buff, end-1);
			}
			else {
				failed++;
			}

			response[0] = '0'+i;

			i++;
			if(i > SEQUENCENUM) {
				i = 0;
			}

			cout << "Sending ack " << response[0] << "... " << endl;

	        send(sock, response, sizeof(response), 0);

			memset(client_message, 0, packetSize+1);
		} else {
			response[0] = client_message[0];
			cout << "Sending ack " << response[0] << "... " << endl;
			send(sock, response, sizeof(response), 0);
		}
	}

	//Last set of write if buffArray got cut short


	if ((i+48) == client_message[0]) {
		// if(i == SEQUENCENUM) { // first reply if needs more info, then write everything
		total++;

		memset(buff, 0, sizeof buff);

		int end = getEndIndex(client_message, packetSize);

		if (end != 0) {
			writes++;
			copy(client_message+1, client_message + end, buff);
			myfile.write((char*) &buff, end-1);
		}
		else {
			failed++;
		}

		response[0] = '0'+i;

		i++;
		if(i > SEQUENCENUM) {
			i = 0;
		}

		cout << "Sending ack " << response[0] << "... " << endl;

        send(sock, response, sizeof(response), 0);

		memset(client_message, 0, packetSize+1);
	} else {
		response[0] = client_message[0];
		cout << "Sending ack " << response[0] << "... " << endl;
		send(sock, response, sizeof(response), 0);
	}


	myfile.close();
	close(sock);

	t2 = clock();
	float diff ((float)t2-(float)t1);
	float seconds = diff / CLOCKS_PER_SEC;


    cout << "\n\n\n" << endl;
	cout << "total ACKs dropped: " << failed << endl;
	cout << "writes to file: " << writes << endl;
    cout << "Number of Packets: " << total << endl;
    cout << "file size: " << filesize("mylogServer.txt") << " bytes" << endl;
    cout << "total elapsed time: " << seconds << endl;
	cout << "MD5SUM: " << exec("md5 mylogServer.txt") << endl;
	// cout << "MD5SUM: " << exec("md5sum mylogServer.txt") << endl;
}

int main(int argc, char *argv[])
{
	stopAndWait();
	//slidingWindow();

	return 0;
}
