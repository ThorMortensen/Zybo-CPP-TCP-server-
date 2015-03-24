/* 
 * File:   main.cpp
 * Author: Thor
 *
 * Created on 24. marts 2015, 16:00
 */

#include <cstdlib>
#include <stdint.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

//Can't return character for some strange reson?! Use this instead of std::endl;
static const string ENDL = ("\n\r");

//Print the usage for this program
static const string USAGE = ("Usage: -ip xxx.xxx.xxx.xxx -port xxxx");

/**
 * Argument expected for this program 
 */
static const string ARG1 = ("-ip");
static const string ARG2 = ("-port");

class ProgArg_s{
public: 
    //ASK FOR 
    ProgArg_s(uint8_t no, const string literal,const string argVal):
    number(no), ARG_LITERAL(literal),ARG_VAL(argVal) {};
    uint8_t number;
    const string ARG_LITERAL;
    const string ARG_VAL;
};


string ipAdressServer(15, 0);
uint16_t portNo = 0;

ProgArg_s arg1(1,"This is a test","this is argval");

/*
 * 
 */
int main(int argc, char** argv) {
    
    cout<<arg1.ARG_LITERAL<<ENDL;
    
    
    //================== Argument Checks ===================
    if (argc < 3 || argc > 3) {
        cout << "Wrong number of arguments" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    if (ARG1 != argv[1]) {
       // ipAdressServer.argv[1+1]
        
    } else {
        cout << "Wrong argument expected -ip" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    if (ARG2 == argv[2]) {
       portNo = atoi(argv[2+1]);  
        
    } else {
        cout << "Wrong argument expected -port" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    //================== Argument Checks end  ===================



    return 0;
}

