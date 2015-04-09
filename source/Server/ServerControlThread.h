/*--------------------------------------------------------------
-- SOURCE FILE: ServerControlThread.h
--
-- DESIGNER: Eric Tsang
--
-- NOTES:
-- Contains the logic of the server.
--------------------------------------------------------------*/
#ifndef SERVERCONTROLTHREAD_H_
#define SERVERCONTROLTHREAD_H_

#include "../Buffer/MessageQueue.h"
#include "Playlist.h"
#include "../protocol.h"
#include "ServerWindow.h"

class UDPSocket;
class FileTransferer;

#define IP_ADDR_LEN 16

class ServerControlThread
{
public:
    static ServerControlThread * getInstance();

    void addConnection(TCPSocket* connection);

    void start();
    void stop();

    void setPlaylist( Playlist * );
    Playlist * getPlaylist();

    void sendPlaylistToAll( void );

    void setUDPSocket( UDPSocket * );
    void setWindow( ServerWindow * );

protected:
    ServerControlThread();
    ~ServerControlThread();

private:

    static DWORD WINAPI _threadRoutine( void * params );
    static DWORD WINAPI _multicastRoutine( void * params );

    static DWORD WINAPI _sendFileToOne( void * params );

    void _handleMsgChangeStream( RequestPacket *, TCPSocket * );
    void _handleMsgRequestDownload( RequestPacket *, TCPSocket* socket);
    void _handleMsgCancelDownload( RequestPacket *, TCPSocket* socket );
    void _handleMsgDisconnect( int clientIndex );

    static VOID CALLBACK _sendPlaylistToAllRoutine( ULONG_PTR );
    static VOID CALLBACK _sendPlaylistToOne( ULONG_PTR tcpSock );

    /**
     * handle to the {ServerWindow}
     */
    ServerWindow * _window;

    /**
     * pointer to the UDPSocket owned by the control thread.
     */
    UDPSocket * udpSocket;

    /**
     * Message queue used to communicate to {_thread}, and get it to do tasks.
     */
    MessageQueue _msgq;

    std::vector< TCPSocket * > _socks;
    std::vector< HANDLE > _sockHandles;

    FileTransferer* fileTransferer;

    /**
     * handle to the thread used to run {ServerControlThread::_threadRoutine}.
     */
    HANDLE _thread;

    /**
     * handle to the thread used to run {ServerControlThread::_multicastRoutine}.
     */
    HANDLE _multicastThread;

    /**
     * handle to an event object, used to stop the execution of thread.
     */
    HANDLE _threadStopEv;

    /**
     * protects the interface functions of the {ServerControlThread}.
     */
    HANDLE access;

    Playlist * playlist;

    /**
     * {SongName} structure of the song that's currently being played.
     */
    SongName* currentsong;
};

#endif
