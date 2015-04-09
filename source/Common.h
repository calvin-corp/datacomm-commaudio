
#ifndef _COMMON_H_
#define _COMMON_H_

#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#include "synchronizationHelper.h"
#include "Client/Sockets.h"


#pragma warning(disable:4996)

/**
 * packet type used for streaming music audio
 */
#define MUSICSTREAM '1'

 /**
  * packet type used for streaming microphone audio
  */
#define MICSTREAM '2'

/**
 * packet type indicating to change the music stream. payload of this kind of
 *   packet is the {RequestPacket}
 */
#define CHANGE_STREAM '3'

/**
 * packet type send along with the {SongName} structure, telling the client to
 *   add a new song to their play list.
 */
#define NEW_SONG '4'

/**
 * packet type indicating the the packet contains information about a download
 *   of a song.
 */
#define DOWNLOAD '5'

/**
 * packet type requesting for a download. payload of this kind of packet is the
 *   {RequestPacket}
 */
#define REQUEST_DOWNLOAD '6'

/**
 * packet type indicating to cancel the download of the specified file. payload
 *   of this kind of packet is the {RequestPacket}
 */
#define CANCEL_DOWNLOAD '7'

/**
 * unused
 */
#define ACTUAL_MUSIC '8'

/**
 * message sent to the server from clients immediately before disconnecting.
 */
#define DISCONNECT '9'

#define WM_SEEK (WM_USER + 22)

#endif
