#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <boost/crc.hpp>


//methods.h
#include "methods.h"
#include "socketMethods.h"

using namespace std;

/// Variable declaration ///
const int packetSize = PACKETSIZE;
const int buffSize = packetSize-10;
char client_message[packetSize+1] = {0};
// memset(client_message, '\0', sizeof client_message);
char buff[buffSize]={0};
char chksum[9] = {0};
char chksumCompare[9] = {0};
char response[1] = {'0'};
int ackNum = 0;
int total = 0;
int sock = 0;
int checksumFails = 0;
bool didChecksumFail = false;
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
		    didChecksumFail = strcmp(chksum, chksumCompare) == 0;
		    if(didChecksumFail) {
					myfile.write((char*) &buff, end-9);
					response[0] = '0'+ackNum;
					ackNum = (ackNum+1) % 2;
			  }
				else {
					cout << "Checksum did not match" << endl;
					checksumFails++;
				}

			}

			memset(client_message, '\0', sizeof client_message); // clear client_message
			memset(chksum, '\0', sizeof chksum);
			memset(chksumCompare, '\0', sizeof chksumCompare);

		}

		if(didChecksumFail) {
			shouldFail = sendACK(sock, response);

			if (shouldFail == false) {
				total++;
			} else {
				failed++;
			}
		}
	}


	myfile.close();
	close(sock);

	t2 = clock();
	float diff ((float)t2-(float)t1);
	float seconds = diff / CLOCKS_PER_SEC;


    cout << "\n\n\n" << endl;
	cout << "total checksums failed: " << checksumFails << endl;
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
char strCpy[packetSize+1] = {0};
clock_t t1, t2;
int failed = 0;
int writes = 0;
int i = 0;
bool completed = false;
mutex mtx;

void SWRecv() {
	while(recv(sock, client_message, packetSize, 0) > 0) {

		mtx.lock();
		cout << "recieved " << total << " [" << client_message[0] << "]\texpecting " << i << endl;
		// if ((i+48) == client_message[0]) {
			buffArr.push_back(client_message);
		// }
		memset(client_message, 0, packetSize+1);
		mtx.unlock();
	}	

	completed = true;
}

void SWSend(ofstream *myfile) {
	while(completed != true || buffArr.size() != 0) {

		mtx.lock();
		if(buffArr.size() != 0) {
			if ((i+48) == buffArr.at(0)[0]) {
			// if(i == SEQUENCENUM) { // first reply if needs more info, then write everything
				total++;

				memset(buff, 0, sizeof buff);

				strcpy(strCpy, buffArr.at(0).c_str());

				int end = getEndIndex(strCpy, packetSize);


				if (end != 0) {
					copy(strCpy + 1, strCpy + end-8, buff);
					copy(strCpy + end-8, strCpy + end, chksum);


					checksum(buff, chksumCompare);
				    didChecksumFail = strcmp(chksum, chksumCompare) == 0;
				    if(didChecksumFail) {
							myfile->write((char*) &buff, end-9);
							writes++;			

					 }
					else {
						cout << "chksum: " << chksum << "\nchksumCompare: " << chksumCompare << endl;
						cout << "Checksum did not match" << endl;
						checksumFails++;
					}
				}


				response[0] = '0'+i;

				i++;
				if(i > SEQUENCENUM) {
					i = 0;
				}

				memset(chksum, '\0', sizeof chksum);
				memset(chksumCompare, '\0', sizeof chksumCompare);
				memset(strCpy, 0, sizeof strCpy);
		    	buffArr.erase(buffArr.begin());

		    	if(shouldFail() == true) {
		    		cout << "Fake Error: ACK Lost" << endl;
		    		failed++;
		    	}else {
					cout << "Sending ack " << response[0] << "... " << endl << endl;
			        send(sock, response, sizeof(response), 0);				
		    	}
			} else {
				if(shouldFail() == true) {
		    		cout << "Fake Error: ACK Lost" << endl;
		    		failed++;
		    	}else {
		    		response[0] = buffArr.at(0)[0];
					cout << "Sending ack " << response[0] << "... " << endl << endl;
			        send(sock, response, sizeof(response), 0);		
			        buffArr.erase(buffArr.begin());		
		    	}
			}
		}
		mtx.unlock();
	}
}

void SlidingWindow() {
		//setup file
	ofstream myfile;
	myfile.open("mylogServer.txt", ios::binary);
	
	if (!myfile.is_open())
    {
    	cout << "file isn't open" << endl;
    }

	
	t1 = clock();


	//Last set of write if buffArray got cut short

	thread recv(SWRecv);
	thread send(SWSend, &myfile);

	recv.join();
	send.join();	


	myfile.close();
	close(sock);

	t2 = clock();
	float diff ((float)t2-(float)t1);
	float seconds = diff / CLOCKS_PER_SEC;


    cout << "\n\n\n" << endl;
	cout << "total ACKs dropped: " << failed << endl;
	cout << "writes to file: " << writes << endl;
    cout << "number of packets: " << total << endl;	
    cout << "file size: " << filesize("mylogServer.txt") << " bytes" << endl;	
    cout << "total elapsed time: " << seconds << endl;	
	cout << "MD5SUM: " << exec("md5 mylogServer.txt") << endl;	
	// cout << "MD5SUM: " << exec("md5sum mylogServer.txt") << endl;
}

int main(int argc, char *argv[])
{
	string protocolType;
	string packetSize;
	string timeout;

	cout << "\tSelect a protocol" << endl;
	cout << "1. Stop and Wait\n" << "2. Sliding Window" << endl;
	cin >> protocolType;

	int protocolTypeInt = stoi(protocolType);

	sock = serverStart();

	if(protocolTypeInt == 1) {
		cout << "\n\nSTOP AND WAIT\n\n";
		stopAndWait();
	}
	else {
		cout << "\n\nSLIDING WINDOW\n\n";
		SlidingWindow();
	}

	return 0;
}
