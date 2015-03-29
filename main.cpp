/* 
 * File:   main.cpp
 * Author: Thor
 *
 * Created on 24. marts 2015, 16:00
 */

#include <stdint.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>//Dono

//For network 
#include <pthread.h>    //For multithreading 
#include <arpa/inet.h>  //For 'inet_ntoa' to get readble ip 
#include <cstring>      //For memset
#include <sys/socket.h> //For sockets (dho)
#include <netdb.h>      //For some socket stuff...Dono
#include <errno.h>      //For error msg's

#include "ProgArg_s.h"


#define PATH_ARG 1
#define EXPECTED_NO_OF_ARGS 4  +PATH_ARG
#define NO_FLAGS 0
#define EXPECTED_CLIENTS 10

using namespace std;

//Can't return character for some strange reson?! Use this instead of std::endl;
static const string ENDL = ("\n\r");

//Print the usage for this program
static const string USAGE = ("Usage: -ip xxx.xxx.xxx.xxx -port xxxx");


//Argument expected for this program 
ProgArg_s ipAddresForThisServer(1, "-ip", STRING, 14);
ProgArg_s portNrForThisServer(2, "-port", NUMBER);
std::vector<ProgArg_s*> args_v(2);

int main(int argc, char** argv) {

    int32_t errorCode = 0;
    int32_t socketId = 0;

    args_v[0] = &ipAddresForThisServer;
    args_v[1] = &portNrForThisServer;

    struct addrinfo hostInfo;
    struct addrinfo* hostInfoList;

    memset(&hostInfo, 0, sizeof (hostInfo));

    //================== Argument Checks ===================
    if (argc < EXPECTED_NO_OF_ARGS || argc > EXPECTED_NO_OF_ARGS) {
        cout << "Wrong number of arguments" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    // O(n^2)   :-)
    for (uint8_t i = 0; i < args_v.size(); i++) {//Get arguments  
        uint8_t j;
        for (j = 1; j < argc; j++) {//0.arg is always path. Don't check that 
            if (args_v[i]->equals(argv[j])) {
                if (!args_v[i]->isValid(argv[j + 1])) {
                    args_v[i]->printError();
                    cout << USAGE << ENDL;
                    return false;
                }
                break;
            }
        }
        if (!args_v[i]->hasValue) {
            args_v[i]->printError();
            cout << USAGE << ENDL;
        }
    }
    //Address info : address = Address field unspecifed (both IPv4 & 6)
    hostInfo.ai_addr = AF_UNSPEC;

    // Address info : socket type. (Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.)
    hostInfo.ai_socktype = SOCK_STREAM;

    hostInfo.ai_flags = AI_PASSIVE; //From .h file: "Socket address is intended for `bind'." 

    //getaddrinfo is used to get info for the socket.
    //Null: use local host. Port no is set by user args (5555)
    errorCode = getaddrinfo(NULL, portNrForThisServer.getParamVal().c_str(), &hostInfo, &hostInfoList);
    if (errorCode != 0) std::cout << "getaddrinfo error" << gai_strerror(errorCode);


    //Make a socket and returns a socket descriptor. 
    //All info comes from 'getaddrinfo' --> into the struct 'hostInfoList'
    socketId = socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    if (socketId == -1) std::cout << "Socket error" << strerror(errno);


    //Socket options is set to reuse the add: http://pubs.opengroup.org/onlinepubs/7908799/xns/setsockopt.html
    //This is to make sure the port is not in use by a previous call by this code. 
    int optionValue_yes = 1;
    errorCode = setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &optionValue_yes, sizeof optionValue_yes);
    //Bind socket to local port.
    errorCode = bind(socketId, hostInfoList->ai_addr, hostInfoList->ai_addrlen);
    if (errorCode == -1) std::cout << "Bind error" << strerror(errno);

    errorCode = listen(socketId, EXPECTED_CLIENTS);
    if (errorCode == -1) std::cout << "Listen error" << strerror(errno);


    cout << "Waiting for incoming data.... Hooooold it ! .... Hooold it!!!" << ENDL;
    int newIncommingSocket = 0;
    struct sockaddr_in incommingAddr;
    socklen_t addrSize = sizeof (incommingAddr);
    newIncommingSocket = accept(socketId, (struct sockaddr*) &incommingAddr, &addrSize);
    if (newIncommingSocket == -1) std::cout << "Accept error" << strerror(errno);
    else cout << "Connection accepted. From : " << inet_ntoa(incommingAddr.sin_addr) << ENDL;



    char rxBuffer [1000]; //Try using cpp strings instead 
    ssize_t bytesRx = recv(newIncommingSocket, rxBuffer, 1000, NO_FLAGS);

    if (bytesRx == 0) std::cout << "host connection shut down." << ENDL;
    if (bytesRx == -1)std::cout << "Rx error!" << strerror(errno) << ENDL;
    rxBuffer[bytesRx] = 0; //Set termeneating  0
    cout << bytesRx << " bytes recieved :" << ENDL;
    cout << rxBuffer << ENDL;

    cout << "Respondig to msg" << ENDL;

    string msg = "Recived thank you closing down";
    ssize_t bytesSent = send(newIncommingSocket, msg.c_str(), msg.length(), NO_FLAGS);
    if (bytesSent != msg.length())std::cout << "Send error. Bytes lost";

    freeaddrinfo(hostInfoList);
    close(socketId);
    close(newIncommingSocket);

    return 0;
}
