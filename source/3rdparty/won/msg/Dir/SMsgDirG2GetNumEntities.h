#ifndef _SMsgDirG2GetNumEntities_H
#define _SMsgDirG2GetNumEntities_H

// SMsgDirG2GetNumEntities.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg {

class SMsgDirG2GetNumEntities : public SmallMessage
{
public:
	// Types
	enum DirGetMode { ServiceCount=0, SubDirCount=1, TotalCount=2 };

	// Wanted to use std::pair here, but VC5 can't handle a wstring in a pair.
	// Might want to change it if VC ever follows the standard.  (SIGH)
	struct DirPathData
	{
		std::wstring mPath;
		DirGetMode   mMode;
	};
	typedef std::list<DirPathData> DirPathList;

	// Default ctor
	SMsgDirG2GetNumEntities(void);

	// SmallMessage ctor
	explicit SMsgDirG2GetNumEntities(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2GetNumEntities(const SMsgDirG2GetNumEntities& theMsgR);

	// Destructor
	~SMsgDirG2GetNumEntities(void);

	// Assignment
	SMsgDirG2GetNumEntities& operator=(const SMsgDirG2GetNumEntities& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Set of paths access
	const DirPathList& Paths(void) const;
	DirPathList&       Paths(void);

private:
	DirPathList mPaths;  // List of directory paths/modes
};


class SMsgDirG2GetNumEntitiesReply : public SmallMessage
{
public:
	// Types
	typedef std::pair<short, unsigned short> NumEntriesData;
	typedef std::list<NumEntriesData> NumEntriesList;

	// Default ctor
	SMsgDirG2GetNumEntitiesReply(void);

	// SmallMessage ctor
	explicit SMsgDirG2GetNumEntitiesReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2GetNumEntitiesReply(const SMsgDirG2GetNumEntitiesReply& theMsgR);

	// Destructor
	~SMsgDirG2GetNumEntitiesReply(void);

	// Assignment
	SMsgDirG2GetNumEntitiesReply& operator=(const SMsgDirG2GetNumEntitiesReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// List access (const and non-const versions)
	const NumEntriesList& Entries() const;
	NumEntriesList&       Entries();

private:
	NumEntriesList mEntries;  // List of NumEntriesData
};


// Inlines
inline TRawMsg*
SMsgDirG2GetNumEntities::Duplicate(void) const
{ return new SMsgDirG2GetNumEntities(*this); }

inline const SMsgDirG2GetNumEntities::DirPathList&
SMsgDirG2GetNumEntities::Paths(void) const
{ return mPaths; }

inline SMsgDirG2GetNumEntities::DirPathList&
SMsgDirG2GetNumEntities::Paths(void)
{ return mPaths; }

inline TRawMsg*
SMsgDirG2GetNumEntitiesReply::Duplicate(void) const
{ return new SMsgDirG2GetNumEntitiesReply(*this); }

inline const SMsgDirG2GetNumEntitiesReply::NumEntriesList&
SMsgDirG2GetNumEntitiesReply::Entries() const
{ return mEntries; }

inline SMsgDirG2GetNumEntitiesReply::NumEntriesList&
SMsgDirG2GetNumEntitiesReply::Entries()
{ return mEntries; }

};  // Namespace WONMsg

#endif