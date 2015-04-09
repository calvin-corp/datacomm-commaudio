/*--------------------------------------------------------------
-- SOURCE FILE: Server.cpp
--
-- DESIGNER: Georgi Hristov
--
-- NOTES:
-- The {Server} class provides network connection utilities
--------------------------------------------------------------*/
#ifndef SERVER_H
#define SERVER_H

#include <ws2tcpip.h>
#include <Winsock2.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "../common.h"
using namespace std;

class Server;

struct WavSong
{
	char* data;
	unsigned long len;
};

struct TCPConnection
{
    SOCKET sock;
    WSAEVENT signal;
    // add more connection data at will
};

typedef void (*newConnectionHandler)( TCPConnection *, void * );


class Server
{
public:
    Server( unsigned short _tcpPort, newConnectionHandler _handler, void * _data, unsigned long groupIP, unsigned short udpPort );
    virtual ~Server();
    
    bool startTCP();
    friend newConnectionHandler;
    
    bool startUDP();
    
    void submitCompletionRoutine( PAPCFUNC lpCompletionRoutine, void * to );
    
	void disconnect();

    friend DWORD WINAPI WorkerThread( LPVOID lpParam );
private:
    unsigned short tcpPort;
    SOCKET listenSocket;

	bool stopSending;
    
    WSAEVENT newConnectionEvent;
	short channels;
	short bitrate;
	unsigned long sampling;
    
    int numTCPConnections;
    TCPConnection * TCPConnections;
    newConnectionHandler handler;
    void * data;
    
    static DWORD WINAPI AcceptThread( LPVOID lpParam );
    HANDLE hAcceptThread;
    static DWORD WINAPI WorkerThread( LPVOID lpParam );
    HANDLE hWorkerThread;
    
    struct sockaddr_in group;
    SOCKET multicastSocket;
};

#endif