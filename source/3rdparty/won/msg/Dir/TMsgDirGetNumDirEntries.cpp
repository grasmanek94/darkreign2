// TMsgDirGetNumDirEntries.cpp

// Directory Get Number of Directory Entries message class.  Sends a list of
// directory paths to the Directory Server.

// Directory Get Number of Directory Entries Reply Message class.  Contains data
// from the Directory Server as a response to a GetNumDirEntries request.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesDir.h"
#include "TMsgDirGetNumDirEntries.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::TMsgDirGetNumDirEntries;
	using WONMsg::TMsgDirGetNumDirEntriesReply;
};


// ** TMsgDirGetNumDirEntries **

// ** Constructors / Destructor

// Default ctor
TMsgDirGetNumDirEntries::TMsgDirGetNumDirEntries(void) :
	TMessage(),
	mEntries()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetNumDirEntries);
}


// TMessage ctor
TMsgDirGetNumDirEntries::TMsgDirGetNumDirEntries(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mEntries()
{
	Unpack();
}


// Copy ctor
TMsgDirGetNumDirEntries::TMsgDirGetNumDirEntries(const TMsgDirGetNumDirEntries& theMsgR) :
	TMessage(theMsgR),
	mEntries(theMsgR.mEntries)
{}


// Destructor
TMsgDirGetNumDirEntries::~TMsgDirGetNumDirEntries(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirGetNumDirEntries&
TMsgDirGetNumDirEntries::operator=(const TMsgDirGetNumDirEntries& theMsgR)
{
	TMessage::operator=(theMsgR);
	mEntries = theMsgR.mEntries;
	return *this;
}


// TMsgDirGetNumDirEntries::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirGetNumDirEntries::Pack(void)
{
	WTRACE("TMsgDirGetNumDirEntries::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetNumDirEntries);
	TMessage::Pack();

	WDBG_LL("TMsgDirGetNumDirEntries::Pack Appending message data");
	AppendShort(mEntries.size());

	WDBG_LL("TMsgDirGetNumDirEntries::Pack Writing " << mEntries.size() << " entries.");
	DirPathList::iterator anItr(mEntries.begin());
	for (; anItr != mEntries.end(); anItr++)
	{
		Append_PW_STRING(anItr->mPath);
		AppendShort(static_cast<short>(anItr->mMode));
	}

	return GetDataPtr();
}


// TMsgDirGetNumDirEntries::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirGetNumDirEntries::Unpack(void)
{
	WTRACE("TMsgDirGetNumDirEntries::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirGetNumDirEntries))
	{
		WDBG_AH("TMsgDirGetNumDirEntries::Unpack Not a DirGetNumDirEntries message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirGetNumDirEntries message.");
	}

	WDBG_LL("TMsgDirGetNumDirEntries::Unpack Reading message data");
	unsigned short anEntryCt = ReadShort();

	WDBG_LL("TMsgDirGetNumDirEntries::Unpack Reading " << anEntryCt << " entries.");
	mEntries.clear();
	for (int i=0; i < anEntryCt; i++)
	{
		DirPathData anEntry;
		ReadWString(anEntry.mPath);
		anEntry.mMode = static_cast<DirGetMode>(ReadShort());
		mEntries.push_back(anEntry);
	}
}


// ** TMsgDirGetNumDirEntriesReply **

// ** Constructors / Destructor

// Default ctor
TMsgDirGetNumDirEntriesReply::TMsgDirGetNumDirEntriesReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mEntries()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetNumDirEntriesReply);
}


// TMessage ctor
TMsgDirGetNumDirEntriesReply::TMsgDirGetNumDirEntriesReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mEntries()
{
	Unpack();
}


// Copy ctor
TMsgDirGetNumDirEntriesReply::TMsgDirGetNumDirEntriesReply(const TMsgDirGetNumDirEntriesReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mEntries(theMsgR.mEntries)
{}


// Destructor
TMsgDirGetNumDirEntriesReply::~TMsgDirGetNumDirEntriesReply(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirGetNumDirEntriesReply&
TMsgDirGetNumDirEntriesReply::operator=(const TMsgDirGetNumDirEntriesReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mStatus  = theMsgR.mStatus;
		mEntries = theMsgR.mEntries;
	}
	return *this;
}


// TMsgDirGetNumDirEntriesReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirGetNumDirEntriesReply::Pack(void)
{
	WTRACE("TMsgDirGetNumDirEntriesReply::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetNumDirEntriesReply);
	TMessage::Pack();

	WDBG_LL("TMsgDirGetNumDirEntriesReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	AppendShort(mEntries.size());

	WDBG_LL("TMsgDirGetNumDirEntriesReply::Pack Appending " << mEntries.size() << "entries.");
	NumEntriesList::iterator anItr = mEntries.begin();
	for (; anItr != mEntries.end(); anItr++)
	{
		AppendShort(static_cast<short>(anItr->first));
		AppendShort(anItr->second);
	}

	return GetDataPtr();
}


// TMsgDirGetNumDirEntriesReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirGetNumDirEntriesReply::Unpack(void)
{
	WTRACE("TMsgDirGetNumDirEntriesReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirGetNumDirEntriesReply))
	{
		WDBG_AH("TMsgDirGetNumDirEntriesReply::Unpack Not a DirGetNumDirEntriesReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirGetNumDirEntriesReply message.");
	}

	WDBG_LL("TMsgDirGetNumDirEntriesReply::Unpack Reading message data");
	mStatus = static_cast<ServerStatus>(static_cast<short>(ReadShort()));
	unsigned short anEntryCt = ReadShort();

	WDBG_LL("TMsgDirGetNumDirEntriesReply::Unpack Reading " << anEntryCt << "entries.");
	mEntries.clear();
	for (int i=0; i < anEntryCt; i++)
	{
		NumEntriesData anEntry;
		anEntry.first  = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
		anEntry.second = ReadShort();
		mEntries.push_back(anEntry);
	}
}
