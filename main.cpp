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
#include <fstream>
#include <time.h>

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
#define CMD_AMOUNT 10
#define RX_BUFFER_SIZE 1400
#define DEBUG



using namespace std;

static const uint8_t SENSOR_AMOUNT = 37;

//Can't return character for some strange reson?! Use this instead of std::endl;
static const string ENDL = ("\n\r");

//Print the usage for this program
static const string USAGE = ("Usage: -ip xxx.xxx.xxx.xxx -port xxxx");
static const string REMOTE_USAGE = (
        "Remote Usage --> Send a 'TM20' cmd followed by: \n\r"
        "                 'GSA' =  GET_SONSOR_AMOUNT\n\r"
        "                 'GET_S' = READ_SENSOR_NO followed by the sensor No\n\r"
        "                 'KILL_C' = CLOSE_CONNECTION from server side\n\r"
        "                 'ECHO' = MAKE_UPPERCASE followed by a str to send back in uppercase \n\r"
        "                 'SEN+' = INCREASE_SAMPLERATE followed by sensor No followed by amount\n\r"
        "                 'SEN-' = DECREASE_SAMPLERATE take a guess \n\r"
        "                 'STOP_S' = STOP_SENSOR followed by sensor No\n\r"
        "                 'START_S' = START_SENSOR same  \n\r"
        "                 'STATUS' = GET_BOARD_STATUS\n\r"
        "You must send atleast one cmd but you can send as many you want in one go \n\r");


uint32_t sampleRate = 0;

static const string TM20_CMD = ("TM20");
static const string GET_SONSOR_AMOUNT = ("GSA");
static const string READ_SENSOR_NO = ("GET_S");
static const string CLOSE_CONNECTION = ("KILL_C");
static const string MAKE_UPPERCASE = ("ECHO");
static const string INCREASE_SAMPLERATE = ("SEN+");
static const string DECREASE_SAMPLERATE = ("SEN-");
static const string STOP_SENSOR = ("STOP_S");
static const string START_SENSOR = ("START_S");
static const string GET_BOARD_STATUS = ("STATUS");


const char DELIMITER = ' ';

//Argument expected for this program 
ProgArg_s ipAddresForThisServer(1, "-ip", STRING, 14);
ProgArg_s portNrForThisServer(2, "-port", NUMBER);
std::vector<ProgArg_s*> args_v(2);

//Add a global ,mutex variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
uint16_t threadCount = 0;
bool freeThreadSlots[EXPECTED_CLIENTS];

struct ClientData_s {
    int socketDescripter;
    uint8_t threadId;
    struct sockaddr_in clientAddr;

};


//______________ From the Internet!!!! ________________
//http://coliru.stacked-crooked.com/a/652f29c0500cf195) 
//http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c

void tokenize(std::string str, std::vector<string> &token_v) {
    size_t start = str.find_first_not_of(DELIMITER), end = start;

    while (start != std::string::npos) {
        // Find next occurence of delimiter
        end = str.find(DELIMITER, start);
        // Push back the token found into vector
        token_v.push_back(str.substr(start, end - start));
        // Skip all occurences of the delimiter to find new start
        start = str.find_first_not_of(DELIMITER, end);
    }
} //========== FROM INTERNET END =================

void writeLogToFile(stringstream log) {
    pthread_mutex_lock(&lock);
    //    
    //    ifstream logfile;
    //    
    //    
    //    

    pthread_mutex_unlock(&lock);
}

const string readSensor(uint8_t sensorNo) {

    //___________  FROM INTERNET!!!! ____________ 
    //http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c

    // Get current date/time
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof (buf), "%X. %d-%m-%Y", &tstruct);

    return buf;
    //========== FROM INTERNET END =================
}

void *clientHandlerThread(void *clientSocket) {

    bool keepAlive = true;
    stringstream log;
    stringstream converter;
    vector<string> rxStrings_v;
    string rxString;
    string txMsg;
    char rxBuffer [RX_BUFFER_SIZE];
    bool rxProblem = false;

    struct ClientData_s *thisClientData;

    thisClientData = (struct ClientData_s *) clientSocket;

    while (keepAlive) {
        converter.str(std::string());
        converter.clear();
        txMsg.clear();
        rxString.clear();
        rxStrings_v.clear();
        memset(rxBuffer, 0, sizeof (rxBuffer));


        ssize_t bytesRx = recv(thisClientData->socketDescripter, rxBuffer, RX_BUFFER_SIZE, NO_FLAGS);

        if (bytesRx == 0) {
            log << "Connection from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << " is lost. Closing thread and conection" << ENDL;
            keepAlive = false;
            continue;
        }

        if (bytesRx == -1) {
            log << "Rx error!" << strerror(errno) << ENDL;
            keepAlive = false;
            continue;
        }

#ifdef DEBUG
        int threadId = thisClientData->threadId;
        converter << threadId;
        txMsg += "Hello '";
        txMsg += inet_ntoa(thisClientData->clientAddr.sin_addr);
        txMsg += "' you have client thread:" + converter.str() + ENDL;
        converter.str(std::string());

#endif


        rxString.append(rxBuffer);
        tokenize(rxString, rxStrings_v);

        if (rxStrings_v.at(0) != TM20_CMD) {
            txMsg += "ERROR: No 'TM20' cmd received" + ENDL;
            txMsg += REMOTE_USAGE;
        } else {
            for (uint8_t i = 1; i < rxStrings_v.size(); i++) {

                if (rxStrings_v[i] == GET_SONSOR_AMOUNT) {
                    converter << SENSOR_AMOUNT;
                    txMsg += converter.str() + " Sensors are attached to this board" + ENDL;
                    converter.str(std::string());
                    converter.clear();
                } else if (rxStrings_v[i] == READ_SENSOR_NO) {
                    i++;
                    if (i < rxStrings_v.size()) {//Safety first !!
                        converter << rxStrings_v[i];
                        uint8_t sensorNo;
                        if (converter >> sensorNo) {
                            txMsg += "Sensor No " + rxStrings_v[i] + " reads: ";
                            txMsg += readSensor(sensorNo) + ENDL;
                        } else {
                            txMsg += "ERROR: Read sensor '" + rxStrings_v[i] +
                                    "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                            txMsg += REMOTE_USAGE;
                            break;
                        }
                    } else {
                        txMsg += "ERROR: Need a sensor number to read! What is wrong with you!" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                    converter.str(std::string());
                    converter.clear();
                } else if (rxStrings_v[i] == CLOSE_CONNECTION) {
                    txMsg += "Closing connection from server side. "
                            "Thank you for participating. See you next time! :-D" + ENDL;
                    keepAlive = false;
                } else if (rxStrings_v[i] == MAKE_UPPERCASE) {
                    ++i; //make next string to upper case 
                    if (i < rxStrings_v.size()) {//Safety first !!
                        txMsg += "ECHO '" + rxStrings_v[i] + "' To upper: ";
                        for (uint8_t ch = 0; ch < rxStrings_v[i].length(); ch++) {
                            txMsg += toupper(rxStrings_v[i].at(ch));
                        }
                    } else {
                        txMsg += "ERROR: Need a string to echo! What do you expect?! Miracles?" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                    txMsg += ENDL;
                } else if (rxStrings_v[i] == INCREASE_SAMPLERATE) {
                    ++i;
                    if (i < rxStrings_v.size()) {//Safety first !!
                        converter << rxStrings_v[i];
                        uint8_t sensorNo;
                        if (converter >> sensorNo) {
                            ++i;
                            converter.str(std::string());
                            converter.clear();
                            if (i < rxStrings_v.size()) {//Safety first !!{
                                converter << rxStrings_v[i];
                                uint8_t sampleRate;
                                if (converter >> sampleRate) {
                                    txMsg += "Sensor No " + rxStrings_v[i - 1] + " now has sample rate  ";
                                    txMsg += rxStrings_v[i] + ENDL;
                                } else {
                                    txMsg += "ERROR: Set sample rate to '" + rxStrings_v[i] +
                                            "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                                    txMsg += REMOTE_USAGE;
                                    break;
                                }
                            } else {
                                txMsg += "ERROR: Need a sample rate to set! What is wrong with you!" + ENDL+ ENDL;
                                txMsg += REMOTE_USAGE;
                                break;
                            }

                        } else {
                            txMsg += "ERROR: Set sample rate of sensor '" + rxStrings_v[i] +
                                    "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                            txMsg += REMOTE_USAGE;
                            break;
                        }
                    } else {
                        txMsg += "ERROR: Need a sensor number to set a sample rate to! What is wrong with you!" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                } else if (rxStrings_v[i] == DECREASE_SAMPLERATE) {
                    ++i;
                    if (i < rxStrings_v.size()) {//Safety first !!
                        converter << rxStrings_v[i];
                        uint8_t sensorNo;
                        if (converter >> sensorNo) {
                            ++i;
                            converter.str(std::string());
                            converter.clear();
                            if (i < rxStrings_v.size()) {//Safety first !!{
                                converter << rxStrings_v[i];
                                uint8_t sampleRate;
                                if (converter >> sampleRate) {
                                    txMsg += "Sensor No " + rxStrings_v[i] + " now has sample rate  ";
                                    txMsg += rxStrings_v[i - 1] + ENDL;
                                } else {
                                    txMsg += "ERROR: Set sample rate to '" + rxStrings_v[i] +
                                            "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                                    txMsg += REMOTE_USAGE;
                                    break;
                                }
                            } else {
                                txMsg += "ERROR: Need a sample rate to set! What is wrong with you!" + ENDL+ ENDL;
                                txMsg += REMOTE_USAGE;
                                break;
                            }
                        } else {
                            txMsg += "ERROR: Set sample rate of sensor '" + rxStrings_v[i] +
                                    "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                            txMsg += REMOTE_USAGE;
                            break;
                        }
                    } else {
                        txMsg += "ERROR: Need a sensor number to set a sample rate to! What is wrong with you!" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                } else if (rxStrings_v[i] == STOP_SENSOR) {
                    i++;
                    if (i < rxStrings_v.size()) {//Safety first !!
                        converter << rxStrings_v[i];
                        uint8_t sensorNo;
                        if (converter >> sensorNo) {
                            txMsg += "Sensor No " + rxStrings_v[i - 1] + " is now STOPPED! ";
                            txMsg += readSensor(sensorNo) + ENDL;
                        } else {
                            txMsg += "ERROR: Stop sensor '" + rxStrings_v[i - 1] +
                                    "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                            txMsg += REMOTE_USAGE;
                            break;
                        }
                    } else {
                        txMsg += "ERROR: Need a sensor number to stop! What is wrong with you!" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                    converter.str(std::string());
                    converter.clear();
                } else if (rxStrings_v[i] == START_SENSOR) {
                    i++;
                    if (i < rxStrings_v.size()) {//Safety first !!
                        converter << rxStrings_v[i];
                        uint8_t sensorNo;
                        if (converter >> sensorNo) {
                            txMsg += "Sensor No " + rxStrings_v[i - 1] + " is now STARTED! " + ENDL;
                        } else {
                            txMsg += "ERROR: Start sensor '" + rxStrings_v[i - 1] +
                                    "' is not a number! Pull yourself together!" + ENDL+ ENDL;
                            txMsg += REMOTE_USAGE;
                            break;
                        }
                    } else {
                        txMsg += "ERROR: Need a sensor number to start! What is wrong with you!" + ENDL+ ENDL;
                        txMsg += REMOTE_USAGE;
                        break;
                    }
                    converter.str(std::string());
                    converter.clear();
                } else if (rxStrings_v[i] == GET_BOARD_STATUS) {
                    txMsg += readSensor(1);
                } else {
                    txMsg += "ERROR: Wrong cmd received" + ENDL+ ENDL;
                    txMsg += REMOTE_USAGE;
                    rxProblem = true;
                    break;
                }
            }
        }

        ssize_t bytesSent = send(thisClientData->socketDescripter, txMsg.c_str(), txMsg.length(), NO_FLAGS);
        if (bytesSent != txMsg.length())std::cout << "Send error. Bytes lost";

    }

    close(thisClientData->socketDescripter);
    pthread_mutex_lock(&lock);
    log << "Closing from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << ENDL;
    freeThreadSlots[thisClientData->threadId] = true;
    threadCount--;
    pthread_mutex_unlock(&lock);
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
    //=============== Argument Checks finished ==================


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

    for (;;) {
        if (threadCount < EXPECTED_CLIENTS) {

            cout << "Waiting for incoming data.... Hooooold it ! .... Hooold it!!!" << ENDL;
            int newIncommingSocket = 0;
            struct sockaddr_in incommingAddr;
            socklen_t addrSize = sizeof (incommingAddr);
            newIncommingSocket = accept(socketId, (struct sockaddr*) &incommingAddr, &addrSize);
            if (newIncommingSocket == -1) std::cout << "Accept error" << strerror(errno);
            else {
                cout << "Connection accepted. From : " << inet_ntoa(incommingAddr.sin_addr) << ENDL;
                for (uint8_t freeSlot = 0; freeSlot < EXPECTED_CLIENTS; freeSlot++) {
                    if (freeThreadSlots[freeSlot]) {

                        pthread_mutex_lock(&lock);
                        freeThreadSlots[freeSlot] = false;
                        threadCount++;
                        pthread_mutex_unlock(&lock);

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
