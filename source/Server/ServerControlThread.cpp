/*--------------------------------------------------------------
-- SOURCE FILE: ServerControlThread.h
--
-- PROGRAMMERS: Eric Tsang
--              Georgi Hristov
--              Manuel Gonzales
-- NOTES:
-- Contains the logic of the server.
--------------------------------------------------------------*/

#include "ServerControlThread.h"
#include "../Client/FileTransferer.h"
#include "../GuiLibrary/GuiWindow.h"
#include "../GuiLibrary/GuiListBox.h"

/*
 * message queue constructor parameters
 */
#define MSGQ_CAPACITY 30
#define MSGQ_ELEM_SIZE sizeof(MsgqElement)
#define SOCK_MSGQ_CAPACITY 1000
#define SOCK_MSGQ_ELEM_SIZE sizeof(SockMsgqElement)

/**
 * element that is put into the message queue.
 */
union MsgqElement
{
    int index;
    char string[STR_LEN];
};

/**
 * element that is put into the socket message queue
 */
union SockMsgqElement
{
    char data[DATA_BUFSIZE];
};

/////////////////////////////////////////
// static function forward declaration //
/////////////////////////////////////////
static int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);

/**
 * returns the singleton instance of the {ServerControlThrhead}
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    ServerControlThread * ServerControlThread::getInstance()
 *
 * @return       [description]
 */
ServerControlThread * ServerControlThread::getInstance()
{
    static ServerControlThread * _instance = new ServerControlThread();
    return _instance;
}

/**
 * creates an instance of the {ServerControlThread}
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    ServerControlThread::ServerControlThread()
 */
ServerControlThread::ServerControlThread()
    : _msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    , _socks()
    , access( CreateMutex(NULL, FALSE, NULL) )
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;
	fileTransferer = new FileTransferer(NULL);
	currentsong = NULL;

    _sockHandles.emplace_back( _threadStopEv );
	currentsong = NULL;
}

/**
 * cleans up all the allocated memory of the {ServerControlThread}
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    ServerControlThread::~ServerControlThread()
 */
ServerControlThread::~ServerControlThread()
{
}

/**
 * sets the playlist for the server
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::setPlaylist( Playlist * _playlist )
 *
 * @param        _playlist   sets the playlist for the server
 */
void ServerControlThread::setPlaylist( Playlist * _playlist )
{
    if( _playlist != NULL )
    {
        WaitForSingleObject(access,INFINITE);
        playlist = _playlist;
        ReleaseMutex(access);
    }
}

/**
 * returns the playlist of the server
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    Playlist * ServerControlThread::getPlaylist()
 *
 * @return       playlist saves in the {ServerControlThread}
 */
Playlist * ServerControlThread::getPlaylist()
{
    return playlist;
}

/**
 * sets the UDPSocket used by the {ServerControlThread} to stream audio.
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::setUDPSocket( UDPSocket * sock )
 *
 * @param        sock   [description]
 */
void ServerControlThread::setUDPSocket( UDPSocket * sock )
{
    if( sock != NULL )
    {
        WaitForSingleObject(access,INFINITE);
        udpSocket = sock;
        udpSocket->setGroup(MULTICAST_ADDR,0);
        ReleaseMutex(access);
    }
}

/**
 * invoked when a new client has connected with the server
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::addConnection( TCPSocket * connection )
 *
 * @param        connection   connection structure
 */
void ServerControlThread::addConnection( TCPSocket * connection )
{
    WaitForSingleObject(access,INFINITE);
    _socks.emplace_back( connection );
    _sockHandles.emplace_back( connection->getMessageQueue()->hasMessage );
    QueueUserAPC( _sendPlaylistToOne        // _In_  PAPCFUNC pfnAPC,
                , _thread                   // _In_  HANDLE hThread,
                , (ULONG_PTR) connection ); // _In_  ULONG_PTR dwData
	RequestPacket packet;
	if(currentsong)
	{
		packet.index = currentsong->id;
		connection->Send(CHANGE_STREAM, &packet, sizeof(packet));
	}
    ReleaseMutex(access);
}

/**
 * stgarts the {ServerControlThread}
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::start()
 */
void ServerControlThread::start()
{
    startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

/**
 * stops the {ServerControlThread}
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::stop()
 */
void ServerControlThread::stop()
{
    stopRoutine(&_thread,_threadStopEv);
}

/**
 * threaded routine of the {ServerControlThread}. it dequeues from all relevant
 *   message queues, and handles the dequeued messages.
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    DWORD WINAPI ServerControlThread::_threadRoutine( void * params
 *   )
 *
 * @param        params   pointer to the {ServerControlThread} instance that
 *   this thread is running for
 *
 * @return       exit code
 */
DWORD WINAPI ServerControlThread::_threadRoutine( void * params )
{
    ServerControlThread * thiz = (ServerControlThread *) params;

    int breakLoop = FALSE;
    while(!breakLoop)
    {
        int handleNum = 0;
		handleNum = WaitForMultipleObjectsEx(  thiz->_sockHandles.size()
                                            , &thiz->_sockHandles[ 0 ]
                                            , FALSE
                                            , INFINITE
                                            , TRUE );
		if( handleNum == WAIT_OBJECT_0 + 0 )
		{
            breakLoop = TRUE;
		}
		else if( handleNum > WAIT_OBJECT_0 + 0 && handleNum < WAIT_OBJECT_0 + thiz->_sockHandles.size() )
        {
			TCPSocket * sock = thiz->_socks[ handleNum - 1 ];

            int type;
			TCPPacket packet;
            sock->getMessageQueue()->dequeue( &type, &packet );
            switch( type )
            {
            case CHANGE_STREAM:
				thiz->_handleMsgChangeStream( &packet.requestPacket, sock );
                break;
            case REQUEST_DOWNLOAD:
                thiz->_handleMsgRequestDownload( &packet.requestPacket, sock);
                break;
            case CANCEL_DOWNLOAD:
                thiz->_handleMsgCancelDownload( &packet.requestPacket, sock );
                break;
            case DISCONNECT:
                thiz->_handleMsgDisconnect( handleNum - 1 );
                break;
            }
		}
		else if( handleNum == WAIT_IO_COMPLETION )
		{

		}
		else
		{
			wchar_t errorStr[256] = {0};
			swprintf( errorStr, 256, L"WaitForMultipleObjectsEx() failed: %d\n"
									 L"thiz->_sockHandles.size(): %d", handleNum, thiz->_sockHandles.size() );
			#ifdef DEBUG
			MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
			#endif
			return false;
		}
    }
    return 0;
}

/**
 * handles the change stream message from the socket
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::_handleMsgChangeStream( RequestPacket * data, TCPSocket * sock )
 *
 * @param        data   data of the packet
 * @param        sock   socket that th emessage was received from
 */
void ServerControlThread::_handleMsgChangeStream( RequestPacket * data, TCPSocket * sock )
{
	ServerControlThread * sct = ServerControlThread::getInstance();
    udpSocket->stopSong();
    WaitForSingleObject(_multicastThread,5000);
    DWORD useless;
	SongName * song = playlist->getSong( data->index );
    _multicastThread = CreateThread( 0, 0, _multicastRoutine, song, 0, &useless );

	sockaddr_in sockAddr;
	int uusless = sizeof( sockaddr_in );
	getpeername( sock->sd, (sockaddr *)&sockAddr, &uusless );
	char * FAR ip = inet_ntoa( sockAddr.sin_addr );

	wchar_t * out = new wchar_t[ strlen( ip ) + 2 + wcslen( song->filepath ) + 1 ];

	size_t uuuseless;
	mbstowcs_s( &uuuseless, out, strlen( ip ) + 1, ip, strlen( ip ) + 1 );

	memcpy( out + strlen( ip ), L": ", 2 * sizeof( wchar_t ) );
	memcpy( out + strlen( ip ) + 2, song->filepath, ( wcslen( song->filepath ) + 1 ) * sizeof( wchar_t ) );

	sct->_window->connectedClients->addItem( out, -1 );

	delete [] out;
}

/**
 * starts the download.
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::_handleMsgRequestDownload( RequestPacket * data, TCPSocket* socket )
 *
 * @param        data   data of the packet
 * @param        socket   socket that the message was received from
 */
void ServerControlThread::_handleMsgRequestDownload( RequestPacket * data, TCPSocket* socket )
{
	fileTransferer->sendFile(playlist->getSong( data->index ), socket);
}

/**
 * cancels a currently downloading download
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::_handleMsgCancelDownload( RequestPacket * data, TCPSocket* socket )
 *
 * @param        data   data of the packet
 * @param        socket   socket that the message was received from
 */
void ServerControlThread::_handleMsgCancelDownload( RequestPacket * data, TCPSocket* socket )
{
	fileTransferer->cancelTransfer(playlist->getSong( data->index )->id, socket);
}

/**
 * handles a disconnection message sent from the client
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::_handleMsgDisconnect( int client )
 *
 * @param        client   socket that the message was received from
 */
void ServerControlThread::_handleMsgDisconnect( int client )
{
    WaitForSingleObject( access, INFINITE );
    _socks.erase( _socks.begin() + client );
    _sockHandles.erase( _sockHandles.begin() + client + 1 );
    ReleaseMutex( access );
}

/**
 * sends the playlist to all connected clients
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    void ServerControlThread::sendPlaylistToAll( void )
 */
void ServerControlThread::sendPlaylistToAll( void )
{
    QueueUserAPC( _sendPlaylistToAllRoutine // _In_  PAPCFUNC pfnAPC,
                , _thread                   // _In_  HANDLE hThread,
                , NULL );                   // _In_  ULONG_PTR dwData
}

/**
 * sthread for the {sendPlaylistToAll} function
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    VOID CALLBACK ServerControlThread::_sendPlaylistToAllRoutine(
 *   ULONG_PTR )
 *
 * @param        ULONG_PTR   unused
 */
VOID CALLBACK ServerControlThread::_sendPlaylistToAllRoutine( ULONG_PTR )
{
    ServerControlThread * thiz = ServerControlThread::getInstance();
    for( std::vector< TCPSocket * >::iterator sockit = thiz->_socks.begin()
        ; sockit != thiz->_socks.end()
        ; ++sockit )
    {
        _sendPlaylistToOne( (ULONG_PTR) *sockit );
    }

}

/**
 * sends a playlist to one client, as well as the change stream message so the
 *   client knows which song is curently playing, and can adjust its sample rate
 *   and such as needed
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang, Georgi Hristov
 *
 * @programmer   Eric Tsang, Georgi Hristov
 *
 * @note         none
 *
 * @signature    VOID CALLBACK ServerControlThread::_sendPlaylistToOne( ULONG_PTR data )
 *
 * @param        data   socket to send the data to.
 */
VOID CALLBACK ServerControlThread::_sendPlaylistToOne( ULONG_PTR data )
{
    ServerControlThread * thiz = ServerControlThread::getInstance();
    TCPSocket * sock = (TCPSocket *) data;

    for( std::vector< SongName >::iterator songit = thiz->playlist->playlist.begin()
       ; songit != thiz->playlist->playlist.end()
       ; ++songit )
    {
        sock->Send( NEW_SONG, &(*songit), sizeof( SongName ) );
    }

	RequestPacket packet;
	if(thiz->currentsong)
	{
		packet.index = thiz->currentsong->id;
		sock->Send(CHANGE_STREAM, &packet, sizeof(packet));
	}
}

DWORD WINAPI ServerControlThread::_multicastRoutine( void * params )
{

    ServerControlThread * thiz = ServerControlThread::getInstance();
	thiz->currentsong = (SongName *) params;
    thiz->udpSocket->sendWave( *((SongName *) params), 60, thiz->_socks );
    return 0;
}
/////////////////////////////////////
// static function implementations //
/////////////////////////////////////

int startRoutine( HANDLE                 * thread
                , HANDLE                   stopEvent
                , LPTHREAD_START_ROUTINE   routine
                , void                   * params)
{
    // return immediately if the routine is already running
    if( *thread != INVALID_HANDLE_VALUE )
        return 1;

    // reset the stop event
    ResetEvent( stopEvent );

    // start the thread & return
    DWORD useless;
    *thread = CreateThread( 0, 0, routine, params, 0, &useless );
    return ( *thread == INVALID_HANDLE_VALUE );
}

int stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
    // return immediately if the routine is already stopped
    if(*thread == INVALID_HANDLE_VALUE)
        return 1;

    // set the stop event to stop the thread
    SetEvent( stopEvent );
    WaitForSingleObject( *thread, INFINITE );

    // invalidate thread handle, so we know it's terminated
    *thread = INVALID_HANDLE_VALUE;
    return 0;
}

void ServerControlThread::setWindow( ServerWindow * window )
{
	if( window != NULL )
	{
		_window = window;
	}
}
