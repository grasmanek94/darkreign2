// SMsgDirG2GetNumEntities.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2GetNumEntities.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2GetNumEntities;
	using WONMsg::SMsgDirG2GetNumEntitiesReply;
};


// ** SMsgDirG2GetNumEntities**

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2GetNumEntities::SMsgDirG2GetNumEntities(void) :
	SmallMessage(),
	mPaths()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetNumEntities);
}


// SmallMessage ctor
SMsgDirG2GetNumEntities::SMsgDirG2GetNumEntities(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mPaths()
{
	Unpack();
}


// Copy ctor
SMsgDirG2GetNumEntities::SMsgDirG2GetNumEntities(const SMsgDirG2GetNumEntities& theMsgR) :
	SmallMessage(theMsgR),
	mPaths(theMsgR.mPaths)
{}


// Destructor
SMsgDirG2GetNumEntities::~SMsgDirG2GetNumEntities(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2GetNumEntities&
SMsgDirG2GetNumEntities::operator=(const SMsgDirG2GetNumEntities& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mPaths = theMsgR.mPaths;
	return *this;
}


// SMsgDirG2GetNumEntities::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2GetNumEntities::Pack(void)
{
	WTRACE("SMsgDirG2GetNumEntities::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetNumEntities);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2GetNumEntities::Pack Appending message data");
	AppendShort(mPaths.size());

	WDBG_LL("SMsgDirG2GetNumEntities::Pack Writing " << mPaths.size() << " entries.");
	DirPathList::iterator anItr(mPaths.begin());
	for (; anItr != mPaths.end(); anItr++)
	{
		Append_PW_STRING(anItr->mPath);
		AppendShort(static_cast<short>(anItr->mMode));
	}

	return GetDataPtr();
}


// SMsgDirG2GetNumEntities::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2GetNumEntities::Unpack(void)
{
	WTRACE("SMsgDirG2GetNumEntities::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2GetNumEntities))
	{
		WDBG_AH("SMsgDirG2GetNumEntities::Unpack Not a DirG2GetNumEntities message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2GetNumEntities message.");
	}

	WDBG_LL("SMsgDirG2GetNumEntities::Unpack Reading message data");
	unsigned short aPathCt = ReadShort();

	WDBG_LL("SMsgDirG2GetNumEntities::Unpack Reading " << aPathCt << " entries.");
	mPaths.clear();
	for (int i=0; i < aPathCt; i++)
	{
		DirPathData anEntry;
		mPaths.push_back(anEntry);
		ReadWString(mPaths.back().mPath);
		mPaths.back().mMode = static_cast<DirGetMode>(ReadShort());
	}
}


// ** SMsgDirG2GetNumEntitiesReply **

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2GetNumEntitiesReply::SMsgDirG2GetNumEntitiesReply(void) :
	SmallMessage(),
	mEntries()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetNumEntitiesReply);
}


// SmallMessage ctor
SMsgDirG2GetNumEntitiesReply::SMsgDirG2GetNumEntitiesReply(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mEntries()
{
	Unpack();
}


// Copy ctor
SMsgDirG2GetNumEntitiesReply::SMsgDirG2GetNumEntitiesReply(const SMsgDirG2GetNumEntitiesReply& theMsgR) :
	SmallMessage(theMsgR),
	mEntries(theMsgR.mEntries)
{}


// Destructor
SMsgDirG2GetNumEntitiesReply::~SMsgDirG2GetNumEntitiesReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2GetNumEntitiesReply&
SMsgDirG2GetNumEntitiesReply::operator=(const SMsgDirG2GetNumEntitiesReply& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mEntries = theMsgR.mEntries;
	return *this;
}


// SMsgDirG2PeerConnectReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer and
// sets the new message length.
void*
SMsgDirG2GetNumEntitiesReply::Pack(void)
{
	WTRACE("SMsgDirG2GetNumEntitiesReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetNumEntitiesReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2GetNumEntitiesReply::Pack Appending message data");
	AppendShort(mEntries.size());

	WDBG_LL("SMsgDirG2GetNumEntitiesReply::Pack Appending " << mEntries.size() << " entries.");
	NumEntriesList::iterator anItr = mEntries.begin();
	for (; anItr != mEntries.end(); anItr++)
	{
		AppendShort(anItr->first);
		AppendShort(anItr->second);
	}

	return GetDataPtr();
}


// SMsgDirG2GetNumEntitiesReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2GetNumEntitiesReply::Unpack(void)
{
	WTRACE("SMsgDirG2GetNumEntitiesReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2GetNumEntitiesReply))
	{
		WDBG_AH("SMsgDirG2GetNumEntitiesReply::Unpack Not a DirG2GetNumEntitiesReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2GetNumEntitiesReply message.");
	}

	WDBG_LL("SMsgDirG2GetNumEntitiesReply::Unpack Reading message data");
	unsigned short anEntryCt = ReadShort();

	WDBG_LL("SMsgDirG2GetNumEntitiesReply::Unpack Reading " << anEntryCt << " entries.");
	mEntries.clear();
	for (int i=0; i < anEntryCt; i++)
	{
		NumEntriesData anEntry;
		mEntries.push_back(anEntry);
		mEntries.back().first  = static_cast<short>(ReadShort());
		mEntries.back().second = ReadShort();
	}
}
