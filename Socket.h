/* 
 * File:   Socket.h
 * Author: Thor
 *
 * Created on 24. marts 2015, 16:18
 */

#ifndef SOCKET_H
#define	SOCKET_H
#include <stdio.h>

class Socket {
public:
    Socket(char *ourIpAdd, uint16_t port);
    Socket(const Socket& orig);
    virtual ~Socket();
private:

};

#endif	/* SOCKET_H */

