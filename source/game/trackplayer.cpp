///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Track Player
//
// 4-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "trackplayer.h"
#include "sound.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrackPlayer - Reusable MP3 Playing Class
//


//
// Constructor
//
TrackPlayer::TrackPlayer() : 
  paths(&Path::node), 
  tracks(&Track::node),
  requested(FALSE),
  active(NULL)
{
  // Setup the unique stream name
  Utils::Sprintf(stream.str, stream.GetSize(), "TP::%08X", U32(this));
}


//
// Destructor
//
TrackPlayer::~TrackPlayer()
{
  Clear();
}


//
// Add
//
// Add tracks to the playlist (returns number of tracks added)
//
U32 TrackPlayer::Add(const char *path)
{
  Dir::Find find;

  // Ignore if this path is already in the play list
  if (FindPath(Crc::CalcStr(path)))
  {
    return (0);
  }

  // Count the number of tracks added
  U32 count = 0;

  // Add all sub-dirs first
  if (Dir::FindFirst(find, path, "*", File::Attrib::SUBDIR))
  {
    do
    {
      if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
      {
        // Create the full path name
        FilePath full;
        Dir::PathMake(full, path, find.finddata.name);

        // Add the path to the playlist
        count += Add(full.str);
      }
    } 
    while (Dir::FindNext(find));
  }  

  // Finish find operation
  Dir::FindClose(find); 

  // Only create a record for this path if a track is found
  Path *pathRecord = NULL;

  // Add the tracks from this folder
  if (Dir::FindFirst(find, path, "*.mp3"))
  {
    do
    {
      // Exclude directories
      if (!(find.finddata.attrib & File::Attrib::SUBDIR))
      {
        if (!pathRecord)
        {
          // Create a record for this path
          pathRecord = new Path(path);

          // Add to the path list
          paths.Append(pathRecord);
        }

        // Create a new track record
        Track *track = new Track(*pathRecord, find.finddata.name);

        // Add to the list
        tracks.Append(track);

        // Activate if none
        if (!active)
        {
          active = track;
        }

        // Increment the count
        count++;
      }
    } 
    while (Dir::FindNext(find));
  }  

  // Finish find operation
  Dir::FindClose(find);

  // Return the count
  return (count);
}



//
// Clear
//
// Clear the current playlist
//
void TrackPlayer::Clear()
{
  // Stop playing
  Stop();

  // Clear the active track
  active = NULL;

  // Dispose of records
  tracks.DisposeAll();
  paths.DisposeAll();
}


//
// Play
//
// Play the active track
//
Bool TrackPlayer::Play()
{
  // If no active track, select the first in the list
  if (!active)
  {
    active = tracks.GetHead();
  }

  // Do we have a track
  if (active)
  {
    // Build full path to file
    FilePath full;
    Dir::PathMake(full, active->GetPath(), active->GetName());

    // Start the track playing
    if (Sound::Digital::Stream::Start(stream.str, full.str))
    {
      // Record that a track has been requested
      requested = TRUE;

      // Success
      return (TRUE);
    }
  }

  // Failed
  return (FALSE);
}


//
// StopStream
//
// Stop the stream playing
//
void TrackPlayer::StopStream()
{
  Sound::Digital::Stream::Stop(stream.str);
}


//
// FindPath
//
// Find the given path
//
TrackPlayer::Path * TrackPlayer::FindPath(U32 crc)
{
  for (NList<Path>::Iterator i(&paths); *i; ++i)
  {
    if ((*i)->GetPath().crc == crc)
    {
      return (*i);
    }
  }

  return (NULL);
}


//
// Stop
//
// Stop playing
//
void TrackPlayer::Stop()
{
  // Stop the stream
  StopStream();

  // Clear requested status
  requested = FALSE;
}


//
// NextTrack
//
// Move to the next track
//
Bool TrackPlayer::NextTrack()
{
  // Stop the stream
  StopStream();

  // Get the next track
  if (active)
  {
    active = active->GetNext();
  }

  // Get the first track
  if (!active)
  {
    active = tracks.GetHead();
  }

  // Should we play it
  if (requested)
  {
    // Did we find a track
    if (active)
    {
      // Play it
      Play();
    }
    else
    {
      // No tracks to play
      requested = FALSE;
    }
  }

  // Returns true if found a track
  return (active ? TRUE : FALSE); 
}


//
// PrevTrack
//
// Move to the previous track
//
Bool TrackPlayer::PrevTrack()
{
  // Stop the stream
  StopStream();

  // Get the previous track
  if (active)
  {
    active = active->GetPrev();
  }

  // Get the last track
  if (!active)
  {
    active = tracks.GetTail();
  }

  // Should we play it
  if (requested)
  {
    // Did we find a track
    if (active)
    {
      // Play it
      Play();

      // Success
      return (TRUE);
    }

    // No tracks to play
    requested = FALSE;
  }

  // Returns true if found a track
  return (active ? TRUE : FALSE); 
}


//
// RandomTrack
//
// Move to a random track
//
Bool TrackPlayer::RandomTrack()
{
  // Stop the stream
  StopStream();

  // Clear the active track
  active = NULL;

  // Do we have any tracks
  if (tracks.GetCount())
  {
    // Setup an iterator
    NList<Track>::Iterator i(&tracks);

    // Pick a random track
    i.GoTo(Random::nonSync.Integer(tracks.GetCount()));

    // Set as active
    active = *i;
  }

  // Should we start a track
  if (requested)
  {
    // Did we find a track
    if (active)
    {
      // Start the track
      Play();

      // Success
      return (TRUE);
    }

    // No tracks
    requested = FALSE;
  }

  return (active ? TRUE : FALSE);
}


//
// Playing
//
// Is the active track currently playing
//
Bool TrackPlayer::Playing()
{
  return (active && Sound::Digital::Stream::IsPlaying(stream.str));
}


//
// Poll
//
// Poll for track advancement
//
Bool TrackPlayer::Poll()
{
  // Do we need to play the next stream
  if (requested && !Playing())
  {
    return (NextTrack());
  }

  return (FALSE);
}

