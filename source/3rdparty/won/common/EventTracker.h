#ifndef _EVENT_TRACKER_H_
#define _EVENT_TRACKER_H_

//
// Event Tracker class (and associated classes)
//
// Author:  Lee Olds
//
// This class can keeps track of potential security problems from a
// particular network address or other event.
//
// It keeps a temporary history of events associated with a key,
// and reports whether the key is "Bad" based upon simple rules.
//
#include <vector>
#include <time.h>
#include <map>

namespace WONCommon
{

class Tracker
{
public:
    // If the given number of events occur within
    // the time window, key is banned for the duration.
    Tracker( int    theEventCount,
             time_t theWindowSize,
             time_t theBanDuration)
       : mWindowSize(theWindowSize),
         mBanDuration(theBanDuration),
         mBannedUntil(0),
         mEventVect(theEventCount)
	{}

    bool IsBad()      const;
    bool IsClean()    const; // returns true if the tracker should be removed.
    bool CountEvent();       // returns true if the event causes a ban.
private:
    const time_t mWindowSize;
    const time_t mBanDuration;

    time_t mBannedUntil;

    std::vector<time_t> mEventVect; // note: set size in constructor.
};

inline bool Tracker::IsBad() const
{
    time_t aTimeNow = time(NULL);
    return( mBannedUntil > aTimeNow );
}

// Counts the event.
// Returns true if the event causes a ban.
//
// The first "out of window" time value is replaced.
// If none are "out of window", the ban is invoked,
// and the oldest time value is replaced.
//

inline bool Tracker::CountEvent()
{
    const time_t aTimeNow       = time(NULL);
    const time_t aStartOfWindow = aTimeNow - mWindowSize;

    int  aMin     = 0;

    int  i;
    for( i=0; i < mEventVect.size(); i++ )
    {
        if( mEventVect[i] < mEventVect[aMin] )
            aMin = i;
        if( mEventVect[i] < aStartOfWindow )
            break;
    }
    mEventVect[aMin] = aTimeNow;

    if( i < mEventVect.size() )
        return false;

    mBannedUntil = aTimeNow + mBanDuration;
    return true;
}

// returns true if the tracker is clean, and can
// be removed.
inline bool Tracker::IsClean() const
{
    const time_t aTimeNow       = time(NULL);
    const time_t aStartOfWindow = aTimeNow - mWindowSize;

    if( mBannedUntil > aTimeNow )
        return false;

    for( int i=0; i < mEventVect.size(); i++ )
        if( mEventVect[i] > aStartOfWindow )
            return false;

    return true;
}


//
// future: Could us a multimap, and allow multiple rules
//         for each event.
// 
template< class KeyType >
class EventTracker
{
public:
    EventTracker( int theEventCount     =5,  // five events
                  time_t theWindowSize  =5,  // within 5 seconds
                  time_t theBanDuration =15*60, // bad for 15 minutes
			      const string &theIdString="" )
          : mEventCount(theEventCount),
            mWindowSize(theWindowSize),
            mBanDuration(theBanDuration),
            mIdString(theIdString)
    {}

    ~EventTracker();

    // check if key is currently locked out.
    // Self cleaning version. A little slower,
    // but you don't have to call Clean().
    bool IsBad( const KeyType &theKey,
                string &theString );

    // check if key is currently locked out.
    // Fast version. Doesn't clean out old entries.
    bool IsBad2( const KeyType &theKey,
                string &theString ) const;

    // count failure, lock out if we meet the criteria
    void CountFailure( const KeyType &theKey );

    // collect garbage
    void Clean();
private:
    typedef std::map<KeyType, Tracker*> TrackerMap;
    typedef std::map<KeyType, Tracker*>::iterator TrackerMapIter;
    typedef std::map<KeyType, Tracker*>::const_iterator TrackerMapCIter;

    const int    mEventCount;
    const time_t mWindowSize;
    const time_t mBanDuration;

    TrackerMap mTrackerMap;
	std::string mIdString;
};

template< class KeyType >
inline
EventTracker<KeyType>::~EventTracker()
{
    TrackerMapIter anIter = mTrackerMap.begin();
    while( anIter != mTrackerMap.end() )
    {
        delete anIter->second;
        anIter++;
    }
}

// Loop through map, removing clean entries
template< class KeyType >
inline void
EventTracker<KeyType>::Clean()
{
    TrackerMapIter anIter = mTrackerMap.begin();
    while( anIter != mTrackerMap.end() )
    {
        if( anIter->second->IsClean() )
        {
            delete anIter->second;
            anIter = mTrackerMap.erase( anIter );
        }
        else
        {
            ++anIter;
        }
    }
}

// check if key is currently locked out.
template< class KeyType >
inline bool
EventTracker<KeyType>::IsBad2( const KeyType &theKey,
                string &theString ) const
{
    TrackerMapCIter aTracker = mTrackerMap.find( theKey );

    if( aTracker == mTrackerMap.end() )
        return false; // No record? No lockout.

    return aTracker->second->IsBad();
}

// check if key is currently locked out.
// Remove clean entries as we go
template< class KeyType >
inline bool
EventTracker<KeyType>::IsBad( const KeyType &theKey,
                string &theString )
{
    TrackerMapIter anIter = mTrackerMap.begin();
    while( anIter != mTrackerMap.end() )
    {
        if( anIter->first == theKey )
        {
            return( anIter->second->IsBad() );
        }
        if( anIter->second->IsClean() )
        {
            delete anIter->second;
            anIter = mTrackerMap.erase( anIter );
        }
        else
        {
            ++anIter;
        }
    }
    return false;
}


// count failure, lock out if we meet the criteria
template< class KeyType >
inline void
EventTracker<KeyType>::CountFailure( const KeyType &theKey )
{
    TrackerMapCIter aTracker = mTrackerMap.find( theKey );

    if( aTracker == mTrackerMap.end() )
    {
        mTrackerMap[ theKey ]
            = new Tracker( mEventCount, mWindowSize, mBanDuration );
    }
    else
    {
        if( aTracker->second->CountEvent() )
		{
		    WDBG_AH("EventTracker::CountFailure triggered "
                  << mIdString
				  << std::hex << " EventKey:" << theKey
				  << std::dec << " Time: "    << TimeToString(time(NULL)) );
		}
    }
}

typedef EventTracker<unsigned long> AddrTracker;

};

#endif
