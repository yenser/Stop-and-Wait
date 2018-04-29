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
    return stopAndWaitProtocol(argc, argv);
    // return SlidingWindowProtocol(argc, argv);
}
