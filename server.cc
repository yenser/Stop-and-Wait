#include <stdio.h>
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

int getEndIndex(char* buff, int size) {
	int i = size;
	while(buff[i] == '\0'){
		i--;
	}
	return i;
}

int main(int argc, char *argv[])
{

	/// Variable declaration ///
	const int packetSize = PACKETSIZE;
	const int buffSize = packetSize-2;
	char client_message[packetSize+1] = {0};
	memset(client_message, '\0', sizeof client_message);
	char buff[buffSize]={0};
	char response[1] = {'0'};
	int ackNum = 0;
	int total = 0;
	int sock = serverStart();
	///////////////////////////


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
				copy(client_message + 1, client_message + end, buff);
				myfile.write((char*) &buff, end-1);



				// checksum stuff
				boost::crc_32_type  result;
			    result.process_bytes(buff, sizeof (buff));
    			cout << std::hex << std::uppercase << result.checksum() << endl;
    			// ^ checksum stuff
			



			}

			memset(client_message, '\0', sizeof client_message); // clear client_message
			

			response[0] = '0'+ackNum;
			ackNum = (ackNum+1) % 2;
		}

		shouldFail = sendACK(sock, response, ackNum);		

		if (shouldFail == false) {
			total++;
		} else {
			failed++;
		}



		
		



		//send ack
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

	return 0;
}