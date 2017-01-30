// TMsgDirPeerSynch.h

// Directory Get Directory Contents Message class.  Fetches contents of a
// Directory from the Directory Server.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "DirServerEntry.h"
#include "TMsgDirPeerSynch.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::DirServerEntry;
	using WONMsg::DirServerEntryVector;
	using WONMsg::TMsgDirPeerSynch;
};


// ** TMsgDirPeerSynch **

// ** Constructors / Destructor

// Default ctor
TMsgDirPeerSynch::TMsgDirPeerSynch(void) :
	TMessage(),
	mServId(),
	mEntries()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerSynch);
}


// TMessage ctor
TMsgDirPeerSynch::TMsgDirPeerSynch(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mServId(),
	mEntries()
{
	Unpack();
}


// Copy ctor
TMsgDirPeerSynch::TMsgDirPeerSynch(const TMsgDirPeerSynch& theMsgR) :
	TMessage(theMsgR),
	mServId(theMsgR.mServId),
	mEntries(theMsgR.mEntries)
{}


// Destructor
TMsgDirPeerSynch::~TMsgDirPeerSynch(void)
{}


// ** Private Methods **

void
TMsgDirPeerSynch::PackEntry(const DirServerEntry& theEntry)
{
	WTRACE("TMsgDirPeerSynch::PackEntry");
	WDBG_LL("TMsgDirPeerSynch::PackEntry Packing entry");
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
TMsgDirPeerSynch::UnpackEntry(DirServerEntry& theEntry)
{
	WTRACE("TMsgDirPeerSynch::UnpackEntry");
	WDBG_LL("TMsgDirPeerSynch::UnpackEntry Unpacking entry");
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
TMsgDirPeerSynch&
TMsgDirPeerSynch::operator=(const TMsgDirPeerSynch& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mServId  = theMsgR.mServId;
		mEntries = theMsgR.mEntries;
	}
	return *this;
}


// TMsgDirPeerSynch::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirPeerSynch::Pack(void)
{
	WTRACE("TMsgDirPeerSynch::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerSynch);
	TMessage::Pack();

	WDBG_LL("TMsgDirPeerSynch::Pack Appending message data");
	Append_PA_STRING(mServId);
	AppendShort(mEntries.size());

	WDBG_LL("TMsgDirPeerSynch::Pack Appending " << mEntries.size() << "entries.");
	DirServerEntryVector::iterator anItr = mEntries.begin();
	for (; anItr != mEntries.end(); anItr++)
		PackEntry(*anItr);

	return GetDataPtr();
}


// TMsgDirPeerSynch::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirPeerSynch::Unpack(void)
{
	WTRACE("TMsgDirPeerSynch::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirPeerSynch))
	{
		WDBG_AH("TMsgDirPeerSynch::Unpack Not a DirPeerSynch message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerSynch message.");
	}

	WDBG_LL("TMsgDirPeerSynch::Unpack Reading message data");
	ReadString(mServId);
	unsigned short anEntryCt = ReadShort();

	WDBG_LL("TMsgDirPeerSynch::Unpack Reading " << anEntryCt << "entries.");
	mEntries.clear();
	for (int i=0; i < anEntryCt; i++)
	{
		DirServerEntry anEntry;
		UnpackEntry(anEntry);
		mEntries.push_back(anEntry);
	}
}


unsigned long
TMsgDirPeerSynch::GetBaseSize() const
{
	return (GetHeaderLength() + sizeof(unsigned short) + mServId.size() + sizeof(short));
}
