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
#include <vector>
#include <netdb.h>

#define PATH_ARG 1
#define EXPECTED_NO_OF_ARGS 4+PATH_ARG

using namespace std;

//Can't return character for some strange reson?! Use this instead of std::endl;
static const string ENDL = ("\n\r");

//Print the usage for this program
static const string USAGE = ("Usage: -ip xxx.xxx.xxx.xxx -port xxxx");

/**
 * Class to hold argument to this program.
 * Holds the argument option an parameter. 
 * @param uint8_t no
 * @param const string literal
 */
class ProgArg_s {
public:

    ProgArg_s() {
    }; //Default constructor for arrays

    ProgArg_s(uint8_t number, const string literal) : //ASK FOR POINTER INSTEAD OF COPY STRING????
    number(number), literal(literal) {
        /*EMTY CONSTRUCTER*/
    };

    void setArgumet(uint8_t number, const string literal) {
        this->number = number;
        this->literal = literal;
    }
    uint8_t number;
    string literal;
    string value;
};


string ipAdressServer(15, 0);
uint16_t portNo = 0;

int main(int argc, char** argv) {

    // Argument expected for this program 
    std::vector<ProgArg_s> args_v(2);
    args_v[0].setArgumet(1, "-ip");
    args_v[1].setArgumet(2, "-port");


    //================== Argument Checks ===================
    if (argc < EXPECTED_NO_OF_ARGS || argc > EXPECTED_NO_OF_ARGS) {
        cout << "Wrong number of arguments" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    // O(n^2)   :-)
    for (uint8_t i = 0; i <  args_v.size() ; i++) {//Get arguments  
        uint8_t j;
        for (j = 1; j < argc; j++) {//0.arg is always path. Don't check that 
            if (args_v[i].literal == argv[j]) {
                args_v[i].value += argv[j + 1];
                break;
            }
        }
        if (args_v[i].value.empty()) {
            cout << "expected '" << args_v[i].literal << "' but wasn't found. Did you misspell?" << ENDL;
            cout << USAGE << ENDL;
            return false;
        }
    }




    //    if (ARG1 != argv[1]) {
    //        ipAdressServer.argv[1+1]
    //        
    //    } else {
    //        cout << "Wrong argument expected -ip" << ENDL;
    //        cout << USAGE << ENDL;
    //        return false;
    //    }
    //
    //    if (ARG2 == argv[2]) {
    //       portNo = atoi(argv[2+1]);  
    //        
    //    } else {
    //        cout << "Wrong argument expected -port" << ENDL;
    //        cout << USAGE << ENDL;
    //        return false;
    //    }

    //================== Argument Checks end  ===================



    return 0;
}

