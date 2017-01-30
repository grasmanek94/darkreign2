// TMsgDirGetDirContents

// Directory Get Directory Contents Message class.  Fetches contents of a
// Directory from the Directory Server.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesDir.h"
#include "DirServerEntry.h"
#include "TMsgDirGetDirContents.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::DirServerEntry;
	using WONMsg::DirServerEntryVector;
	using WONMsg::TMsgDirGetDirContents;
	using WONMsg::TMsgDirGetDirContentsReply;
};


// ** TMsgDirGetDirContents **

// ** Constructors / Destructor

// Default ctor
TMsgDirGetDirContents::TMsgDirGetDirContents(void) :
	TMessage(),
	mDirectoryPath(),
	mRecursive(false)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetDirContents);
}


// TMessage ctor
TMsgDirGetDirContents::TMsgDirGetDirContents(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mDirectoryPath(),
	mRecursive(false)
{
	Unpack();
}


// Copy ctor
TMsgDirGetDirContents::TMsgDirGetDirContents(const TMsgDirGetDirContents& theMsgR) :
	TMessage(theMsgR),
	mDirectoryPath(theMsgR.mDirectoryPath),
	mRecursive(theMsgR.mRecursive)
{}


// Destructor
TMsgDirGetDirContents::~TMsgDirGetDirContents(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirGetDirContents&
TMsgDirGetDirContents::operator=(const TMsgDirGetDirContents& theMsgR)
{
	TMessage::operator=(theMsgR);
	mDirectoryPath = theMsgR.mDirectoryPath;
	mRecursive     = theMsgR.mRecursive;
	return *this;
}


// TMsgDirGetDirContents::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirGetDirContents::Pack(void)
{
	WTRACE("TMsgDirGetDirContents::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetDirContents);
	TMessage::Pack();

	WDBG_LL("TMsgDirGetDirContents::Pack Appending message data");
	Append_PW_STRING(mDirectoryPath);
	AppendByte(mRecursive ? 1 : 0);

	return GetDataPtr();
}


// TMsgDirGetDirContents::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirGetDirContents::Unpack(void)
{
	WTRACE("TMsgDirGetDirContents::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirGetDirContents))
	{
		WDBG_AH("TMsgDirGetDirContents::Unpack Not a DirGetDirContents message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirGetDirContents message.");
	}

	WDBG_LL("TMsgDirGetDirContents::Unpack Reading message data");
	ReadWString(mDirectoryPath);
	mRecursive = (ReadByte() == 0 ? false : true);
}


// ** TMsgDirGetDirContentsReply **

// ** Constructors / Destructor

// Default ctor
TMsgDirGetDirContentsReply::TMsgDirGetDirContentsReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mEntries()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetDirContentsReply);
}


// TMessage ctor
TMsgDirGetDirContentsReply::TMsgDirGetDirContentsReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mEntries()
{
	Unpack();
}


// Copy ctor
TMsgDirGetDirContentsReply::TMsgDirGetDirContentsReply(const TMsgDirGetDirContentsReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mEntries(theMsgR.mEntries)
{}


// Destructor
TMsgDirGetDirContentsReply::~TMsgDirGetDirContentsReply(void)
{}


// ** Private Methods **

void
TMsgDirGetDirContentsReply::PackEntry(const DirServerEntry& theEntry)
{
	WTRACE("TMsgDirGetDirContentsReply::PackEntry");
	WDBG_LL("TMsgDirGetDirContentsReply::PackEntry Packing entry");

	AppendByte(theEntry.mType);
	Append_PW_STRING(theEntry.mPath);
	Append_PW_STRING(theEntry.mName);
	Append_PW_STRING(theEntry.mDisplayName);
	AppendLong(theEntry.mLifespan);
	AppendLong(theEntry.mCreated);

	if (theEntry.mType == DirServerEntry::EntryTypeDirectory)
	{
		AppendByte(theEntry.mVisible ? 1 : 0);
	}
	else if (theEntry.mType == DirServerEntry::EntryTypeService)
	{
		Append_PW_STRING(theEntry.mVersion);
		Append_PW_STRING(theEntry.mProtoName);
		Append_PW_STRING(theEntry.mProtoVersion);
		Append_PW_STRING(theEntry.mNetAddress);
		AppendShort(theEntry.mBlobLen);
		AppendBytes(theEntry.mBlobLen, theEntry.mBlob);
	}
}


void
TMsgDirGetDirContentsReply::UnpackEntry(DirServerEntry& theEntry)
{
	WTRACE("TMsgDirGetDirContentsReply::UnpackEntry");
	WDBG_LL("TMsgDirGetDirContentsReply::UnpackEntry Unpacking entry");
	theEntry.mType = ReadByte();
	ReadWString(theEntry.mPath);
	ReadWString(theEntry.mName);
	ReadWString(theEntry.mDisplayName);
	theEntry.mLifespan = ReadLong();
	theEntry.mCreated  = ReadLong();

	if (theEntry.mType == DirServerEntry::EntryTypeDirectory)
	{
		theEntry.mVisible = (ReadByte() == 0 ? false : true);
	}
	else if (theEntry.mType == DirServerEntry::EntryTypeService)
	{
		ReadWString(theEntry.mVersion);
		ReadWString(theEntry.mProtoName);
		ReadWString(theEntry.mProtoVersion);
		ReadWString(theEntry.mNetAddress);
		theEntry.mBlobLen = ReadShort();
		theEntry.SetBlob(ReadBytes(theEntry.mBlobLen), theEntry.mBlobLen);
	}
}


// ** Public Methods

// Assignment operator
TMsgDirGetDirContentsReply&
TMsgDirGetDirContentsReply::operator=(const TMsgDirGetDirContentsReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mStatus  = theMsgR.mStatus;
		mEntries = theMsgR.mEntries;
	}
	return *this;
}


// TMsgDirGetDirContents::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirGetDirContentsReply::Pack(void)
{
	WTRACE("TMsgDirGetDirContentsReply::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirGetDirContentsReply);
	TMessage::Pack();

	WDBG_LL("TMsgDirGetDirContentsReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	AppendShort(mEntries.size());

	WDBG_LL("TMsgDirGetDirContentsReply::Pack Appending " << mEntries.size() << "entries.");
	DirServerEntryVector::iterator anItr = mEntries.begin();
	for (; anItr != mEntries.end(); anItr++)
		PackEntry(*anItr);

	return GetDataPtr();
}


// TMsgDirGetDirContents::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirGetDirContentsReply::Unpack(void)
{
	WTRACE("TMsgDirGetDirContentsReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirGetDirContentsReply))
	{
		WDBG_AH("TMsgDirGetDirContentsReply::Unpack Not a DirGetDirContentsReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirGetDirContentsReply message.");
	}

	WDBG_LL("TMsgDirGetDirContentsReply::Unpack Reading message data");
	mStatus = static_cast<ServerStatus>(static_cast<short>(ReadShort()));
	unsigned short anEntryCt = ReadShort();

	WDBG_LL("TMsgDirGetDirContentsReply::Unpack Reading " << anEntryCt << "entries.");
	mEntries.clear();
	for (int i=0; i < anEntryCt; i++)
	{
		DirServerEntry anEntry;
		UnpackEntry(anEntry);
		mEntries.push_back(anEntry);
	}
}


unsigned long
TMsgDirGetDirContentsReply::GetBaseSize() const
{
	return (GetHeaderLength() + sizeof(short) + sizeof(short));
}
