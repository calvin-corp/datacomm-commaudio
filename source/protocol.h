/**
 * holds definitions of packets exchanged between the client and server.
 *
 * @sourceFile   protocol.h
 *
 * @program      commaudio.exe
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Calvin Rempel, Georgi Hristov, Manuel Gonzales, Eric Tsang
 *
 * @programmer   Calvin Rempel, Georgi Hristov, Manuel Gonzales, Eric Tsang
 *
 * @note         none
 */
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <cstdint>
#include <vector>

#define MULTICAST_ADDR "239.255.0.241"

#define MULTICAST_PORT 7778

#define DATA_BUFSIZE 8196

#define SIZE_INDEX 4

#define STREAM_PACKET 9

#define DATA_LEN 256

#define STR_LEN 128

#define AUDIO_BITS_PER_SAMPLE 8

#define AUDIO_SAMPLE_RATE (44100/2)

#define AUDIO_BUFFER_LENGTH DATA_LEN

#define NUM_AUDIO_CHANNELS 1

#define FILENAME_PACKET_LENGTH 128

#define FILE_PACKET_SIZE 256

/**
 * audio data packet, that has an {index}, describing in what order the packet is
 *   supposed to be played.
 *
 * also has a {data} member, used to hold the raw PCM data
 */
struct DataPacket
{
	int index;
	char data[DATA_LEN];
};

typedef struct DataPacket DataPacket;

/**
 * local data packet is used internal to the client. its like a data packet, but
 *   has an extra member for storing the packet's source address.
 *
 * {index}; describing in what order the packet is supposed to be played in.
 *
 * {srcAddr}; holds the source address that sent this packet
 *
 * {data}; raw PCM data to play
 */
struct LocalDataPacket
{
	int index;
	unsigned long srcAddr;
	char data[DATA_LEN];
};

typedef struct LocalDataPacket LocalDataPacket;

/**
 * packet sent between the client and server for different kinds of requests,
 *   like canceling a download, starting a download, changing the stream, and so
 *   on.
 *
 * {index}; integer that identifies which song is being played, or downloaded.
 */
struct RequestPacket
{
	int index;
};

typedef struct RequestPacket RequestPacket;

struct MessageHeader
{
	uint32_t size;
	uint8_t type;
};

typedef struct MessageHeader MessageHeader;

/**
 * structure that holds all kinds of information about a song within itself.
 *   information included are path to the song, in wide characters, path to the
 *   song in a c style string, name of the song, information about the PCM used
 *   for playing the song, and so on.
 *
 * many of these structures are sent to the client from the server when it
 *   initially connects, so the client can create a song list on it's side.
 *
 * {id}; id used to identify this song
 *
 * {channels}; number of channels in the OCM data
 *
 * {bps}; number of bits per sample of the PCM data
 *
 * {sample_rate}; sampling rate of the PCM data
 *
 * {size}; size of the song in bytes
 *
 * {filepath}; path to the file on the server side in wide characters
 *
 * {cFilepath}; path to the file on the client side
 *
 * {cFilename}; name of the file on the client side
 */
struct SongName
{
	int id;
	short channels;
	short bps; //bits per sample
	unsigned long sample_rate;
	unsigned long size;
	wchar_t filepath[STR_LEN];
	char cFilepath[STR_LEN];
	char cFilename[STR_LEN];
};

typedef struct SongName SongName;

/**
 * payload of the packet exchanged between the client and the server's file
 *   transfer classes
 *
 * {filename}; name of the song file being downloaded
 *
 * {data}; data about the file being downloaded
 *
 * {dataLen}; length of the data in the {data} member
 *
 * {f_SOF}; true if start of frame
 *
 * {f_EOF}; true if the file has reached EOF
 *
 * {songId}; id of the song being downloaded
 */
struct FileTransferData
{
	char filename[FILENAME_PACKET_LENGTH];
	char data[FILE_PACKET_SIZE];
	int dataLen;
	bool f_SOF;
	bool f_EOF;
	int songId;
};

typedef struct FileTransferData FileTransferData;

/**
 * all the packets that the client and server exchange over the TCP control
 *   connection.
 */
union TCPPacket
{
	SongName songName;
	RequestPacket requestPacket;
	DataPacket dataPacket;
	FileTransferData fileTransferData;
};

typedef union TCPPacket TCPPacket;

/*
	Data Sent/Received to communicate file data.
 */

#endif
