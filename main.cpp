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

//Add a global ,mutex variable
pthread_mutex_t mutexClientThreadCount = PTHREAD_MUTEX_INITIALIZER;
uint16_t threadCount = 0;
bool freeThreadSlots[EXPECTED_CLIENTS];

struct ClientData_s {
    int socketDescripter;
    uint8_t threadId;
    struct sockaddr_in clientAddr;

};

void *clientHandlerThread(void *clientSocket) {

    struct ClientData_s *thisClientData;

    thisClientData = (struct ClientData_s *) clientSocket;

    char rxBuffer [1000];
    ssize_t bytesRx = recv(thisClientData->socketDescripter, rxBuffer, 1000, NO_FLAGS);

    if (bytesRx == 0) {
        pthread_mutex_lock(&mutexClientThreadCount);
        cout << "Connection from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << " is lost. Closing thread and conection" << ENDL;
        freeThreadSlots[thisClientData->threadId] = true;
        threadCount--;
        pthread_mutex_unlock(&mutexClientThreadCount);
        pthread_exit(NULL);
    }

    if (bytesRx == -1) {
        pthread_mutex_lock(&mutexClientThreadCount);
        std::cout << "Rx error!" << strerror(errno) << ENDL;
        freeThreadSlots[thisClientData->threadId] = true;
        threadCount--;
        pthread_mutex_unlock(&mutexClientThreadCount);
        pthread_exit(NULL);
    }


    rxBuffer[bytesRx] = 0; //Set termeneating  0

    pthread_mutex_lock(&mutexClientThreadCount);
    cout << bytesRx << " bytes recieved :" << ENDL;
    cout << rxBuffer << ENDL;
    cout << "Respondig to msg" << ENDL;
    pthread_mutex_unlock(&mutexClientThreadCount);

    string msg = "Recived thank you closing down this thread: ";
    stringstream convert;
    int temp = thisClientData->threadId;
    convert << temp;

    msg += convert.str();
    ssize_t bytesSent = send(thisClientData->socketDescripter, msg.c_str(), msg.length(), NO_FLAGS);
    if (bytesSent != msg.length())std::cout << "Send error. Bytes lost";

    close(thisClientData->socketDescripter);

    pthread_mutex_lock(&mutexClientThreadCount);
    cout << "Closing from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << ENDL;
    freeThreadSlots[thisClientData->threadId] = true;
    threadCount--;
    pthread_mutex_unlock(&mutexClientThreadCount);
    pthread_exit(NULL);
    pthread_mutex_unlock(&mutexClientThreadCount);

    pthread_exit(NULL);

}

int main(int argc, char** argv) {

    ClientData_s clientData[EXPECTED_CLIENTS] = {0};
    pthread_t threads[EXPECTED_CLIENTS];


    int32_t errorCode = 0;
    int32_t socketId = 0;

    args_v[0] = &ipAddresForThisServer;
    args_v[1] = &portNrForThisServer;

    struct addrinfo hostInfo;
    struct addrinfo* hostInfoList;

    memset(&hostInfo, 0, sizeof (hostInfo));
    memset(&freeThreadSlots, true, sizeof (freeThreadSlots));


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

    freeaddrinfo(hostInfoList); //Not needed anymore...:-( 

    while (true) {
        if (threadCount <= EXPECTED_CLIENTS) {

            cout << "Waiting for incoming data.... Hooooold it ! .... Hooold it!!!" << ENDL;
            int newIncommingSocket = 0;
            struct sockaddr_in incommingAddr;
            socklen_t addrSize = sizeof (incommingAddr);
            newIncommingSocket = accept(socketId, (struct sockaddr*) &incommingAddr, &addrSize);
            if (newIncommingSocket == -1) std::cout << "Accept error" << strerror(errno);
            else {
                cout << "Connection accepted. From : " << inet_ntoa(incommingAddr.sin_addr) << ENDL;
                for (uint8_t freeSlot = 0; freeSlot <= EXPECTED_CLIENTS; freeSlot++) {
                    if (freeThreadSlots[freeSlot]) {

                        pthread_mutex_lock(&mutexClientThreadCount);
                        freeThreadSlots[freeSlot] = false;
                        threadCount++;
                        pthread_mutex_unlock(&mutexClientThreadCount);

                        clientData[freeSlot].threadId = freeSlot;
                        clientData[freeSlot].clientAddr = incommingAddr;
                        clientData[freeSlot].socketDescripter = newIncommingSocket;

                        pthread_create(&threads[freeSlot], NULL, clientHandlerThread, static_cast<void*> (&clientData[freeSlot]));
                        break;
                    }
                }
            }
        }

    }



    return 0;
}
