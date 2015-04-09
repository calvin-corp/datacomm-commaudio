/*--------------------------------------------------------------
-- SOURCE FILE: Playlist.cpp
--
-- DESIGNER: Georgi Hristov
--
-- NOTES:
-- The {Playlist} class reads the song list and stores it.
--------------------------------------------------------------*/
#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "../common.h"
#include "../protocol.h"

class Playlist
{// friendly !!!
    friend class ServerControlThread;
    friend class ServerWindow; 
public:
    /*
    -- Initiates the {Playlist} by reading the files
    -- in a directory,  specified by a
    -- path which can include wildcards ('*', '?')
    */
    Playlist( wchar_t * _dir );
    
    ~Playlist();
    
    
    /*
    -- Returns the full path to
    -- the song with the given id
    */
    wchar_t * getSongPath( int id );
    
    /*
    -- Returns song info of
    -- the song with the given id
    */
    SongName * getSong( int id );

private:
    // Used to read in the filenames
    WIN32_FIND_DATA   ffd;
    HANDLE            hFind;
    LARGE_INTEGER     filesize;
    
    // stores the path of the parent directory
    wchar_t         * sDir;

    // Stores the playlist
    std::vector< SongName > playlist;
};

#endif