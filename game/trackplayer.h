///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Track Player
//
// 4-APR-2000
//


#ifndef __TRACKPLAYER_H
#define __TRACKPLAYER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrackPlayer - Reusable MP3 Playing Class
//
class TrackPlayer
{
protected:

  // A single path record
  class Path
  {
  public:

    // The tree node
    NList<Path>::Node node;

  protected:

    // The actual path
    PathIdent path;

  public:

    // Constructor
    Path(const char *path) : path(path)
    {
    }

    // Get the path
    const PathIdent & GetPath() const
    {
      return (path);
    }
  };

public:

  // A single track record
  class Track
  {
  public:

    // The list node
    NList<Track>::Node node;

  protected:

    // The path containing this track
    const Path &path;

    // The name of the mp3 file
    PathString name;

  public:

    // Constructor
    Track(const Path &path, const char *name) : path(path), name(name)
    {
    }

    // Returns the path string
    const char * GetPath() const
    {
      return (path.GetPath().str);
    }

    // Returns the file name
    const char * GetName() const
    {
      return (name.str);
    }

    // Get the next track
    Track * GetNext()
    {
      return (node.GetNext() ? node.GetNext()->GetData() : NULL);
    }

    // Get the previous track
    Track * GetPrev()
    {
      return (node.GetPrev() ? node.GetPrev()->GetData() : NULL);
    }
  };

protected:
  
  // Stop the stream playing
  void StopStream();

  // Find the given path
  Path * FindPath(U32 crc);

protected:

  // Stream name unique to this player
  GameString stream;

  // Ordered list of paths
  NList<Path> paths;

  // Ordered list of tracks
  NList<Track> tracks;

  // Has a track currently been requested
  Bool requested;

  // Pointer to the active track
  Track *active;

public:

  // Constructor and destructor
  TrackPlayer();
  ~TrackPlayer();

  // Add tracks to the playlist (returns number of tracks added)
  U32 Add(const char *path);

  // Clear the current playlist
  void Clear();

  // Play the active track
  Bool Play();

  // Stop playing
  void Stop();

  // Move to the next track
  Bool NextTrack();

  // Move to the previous track
  Bool PrevTrack();

  // Move to a random track
  Bool RandomTrack();

  // Is the active track currently playing
  Bool Playing();

  // Poll for track advancement
  Bool Poll();

  // Get the active track
  const Track * GetActive()
  {
    return (active);
  }

  // Get the ordered list of tracks
  const NList<Track> & GetTracks()
  {
    return (tracks);
  }
};

#endif