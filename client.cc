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
#include "SaW.h"
#include "SW.h"

using namespace std;



//main driver program
int main(int argc, char *argv[])
{
	string protocolType;
	string packetSize;
	string timeout;

	cout << "\tSelect a protocol" << endl;
	cout << "1. Stop and Wait\n" << "2. Sliding Window GBN\n" << "3. Sliding Window SR" << endl;
	cin >> protocolType;

	cout << "Timeout: ";
	cin >> timeout;
	int timeoutInt = stoi(timeout);

	int protocolTypeInt = stoi(protocolType);

	if(protocolTypeInt == 1) {
		cout << "\n\nSTOP AND WAIT\n\n";
		stopAndWaitProtocol(argc, argv, timeoutInt);
	}
	else if (protocolTypeInt == 2) {
		cout << "\n\nSLIDING WINDOW GBN\n\n";
		SlidingWindowProtocol(argc, argv, 0, timeoutInt);
	}
	else {
		cout << "\n\nSLIDING WINDOW GBN\n\n";
		SlidingWindowProtocol(argc, argv, 1, timeoutInt);
	}
}
