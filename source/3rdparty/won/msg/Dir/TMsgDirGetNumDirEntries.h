#ifndef _TMsgDirGetNumDirEntries_H
#define _TMsgDirGetNumDirEntries_H

// TMsgDirGetNumDirEntries.h

// Directory Get Number of Directory Entries message class.  Sends a list of
// directory paths to the Directory Server.

// Directory Get Number of Directory Entries Reply Message class.  Contains data
// from the Directory Server as a response to a GetNumDirEntries request.


#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirGetNumDirEntries : public TMessage
{
public:
	// Types
	enum DirGetMode { TotalCount=0, ServiceCount=1, SubDirCount=2 };

	// Wanted to use std::pair here, but VC5 can't handle a wstring in a pair.
	// Might want to change it if VC ever follows the standard.  (SIGH)
	struct DirPathData
	{
		std::wstring mPath;
		DirGetMode   mMode;
	};
	typedef std::list<DirPathData> DirPathList;

	// Default ctor
	TMsgDirGetNumDirEntries(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgDirGetNumDirEntries(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirGetNumDirEntries(const TMsgDirGetNumDirEntries& theMsgR);

	// Destructor
	~TMsgDirGetNumDirEntries(void);

	// Assignment
	TMsgDirGetNumDirEntries& operator=(const TMsgDirGetNumDirEntries& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Set of paths access
	const DirPathList& Entries(void) const;
	DirPathList&       Entries(void);

private:
	DirPathList mEntries;  // List of directory paths
};


class TMsgDirGetNumDirEntriesReply : public TMessage
{
public:
	// Types
	typedef std::pair<ServerStatus, unsigned short> NumEntriesData;
	typedef std::list<NumEntriesData> NumEntriesList;

	// Default ctor
	TMsgDirGetNumDirEntriesReply(void);

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgDirGetNumDirEntriesReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirGetNumDirEntriesReply(const TMsgDirGetNumDirEntriesReply& theMsgR);

	// Destructor
	~TMsgDirGetNumDirEntriesReply(void);

	// Assignment
	TMsgDirGetNumDirEntriesReply& operator=(const TMsgDirGetNumDirEntriesReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Status access
	ServerStatus GetStatus() const;
	void         SetStatus(ServerStatus theStatus);

	// List access (const and non-const versions)
	const NumEntriesList& Entries() const;
	NumEntriesList&       Entries();

private:
	ServerStatus   mStatus;           // Overall status of request
	NumEntriesList mEntries;   // List of NumEntriesData
};


// Inlines
inline TRawMsg*
TMsgDirGetNumDirEntries::Duplicate(void) const
{ return new TMsgDirGetNumDirEntries(*this); }

inline const TMsgDirGetNumDirEntries::DirPathList&
TMsgDirGetNumDirEntries::Entries(void) const
{ return mEntries; }

inline TMsgDirGetNumDirEntries::DirPathList&
TMsgDirGetNumDirEntries::Entries(void)
{ return mEntries; }

inline TRawMsg*
TMsgDirGetNumDirEntriesReply::Duplicate(void) const
{ return new TMsgDirGetNumDirEntriesReply(*this); }

inline ServerStatus
TMsgDirGetNumDirEntriesReply::GetStatus() const
{ return mStatus; }

inline void
TMsgDirGetNumDirEntriesReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const TMsgDirGetNumDirEntriesReply::NumEntriesList&
TMsgDirGetNumDirEntriesReply::Entries() const
{ return mEntries; }

inline TMsgDirGetNumDirEntriesReply::NumEntriesList&
TMsgDirGetNumDirEntriesReply::Entries()
{ return mEntries; }


};  // Namespace WONMsg

#endif