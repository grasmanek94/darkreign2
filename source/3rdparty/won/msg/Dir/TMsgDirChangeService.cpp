// TMsgDirChangeService.h

// DirectoryServer Change Entry message.  Defines an entry to change.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "DirServerSKMasks.h"
#include "TMsgDirServiceBase.h"
#include "TMsgDirChangeService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirServiceBase;
	using WONMsg::TMsgDirChangeService;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirChangeService::TMsgDirChangeService(void) :
	TMsgDirServiceBase(),
	mChangeMask(WONMsg::SKMASK_ALL),
	mNewName(),
	mNewDisplayName(),
	mNewVersion(),
	mNewProtoName(),
	mNewProtoVersion(),
	mNewNetAddress(),
	mNewLifespan(0),
	mNewBlobLen(0),
	mNewBlob(NULL)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirChangeService);
}


// TMessage ctor
TMsgDirChangeService::TMsgDirChangeService(const TMessage& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mChangeMask(WONMsg::SKMASK_ALL),
	mNewName(),
	mNewDisplayName(),
	mNewVersion(),
	mNewProtoName(),
	mNewProtoVersion(),
	mNewNetAddress(),
	mNewLifespan(0),
	mNewBlobLen(0),
	mNewBlob(NULL)
{
	Unpack();
}


// Copy ctor
TMsgDirChangeService::TMsgDirChangeService(const TMsgDirChangeService& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mChangeMask(theMsgR.mChangeMask),
	mNewName(theMsgR.mNewName),
	mNewDisplayName(theMsgR.mNewDisplayName),
	mNewVersion(theMsgR.mNewVersion),
	mNewProtoName(theMsgR.mNewProtoName),
	mNewProtoVersion(theMsgR.mNewProtoVersion),
	mNewNetAddress(theMsgR.mNewNetAddress),
	mNewLifespan(theMsgR.mNewLifespan),
	mNewBlobLen(0),
	mNewBlob(NULL)
{
	SetNewBlob(theMsgR.mNewBlob, theMsgR.mNewBlobLen);
}


// Destructor
TMsgDirChangeService::~TMsgDirChangeService(void)
{
	delete mNewBlob;
}


// ** Public Methods

// Assignment operator
TMsgDirChangeService&
TMsgDirChangeService::operator=(const TMsgDirChangeService& theMsgR)
{
	if (this != &theMsgR)  // protect vs a = a
	{
		delete mNewBlob;  mNewBlob = NULL;
		TMsgDirServiceBase::operator=(theMsgR);
		mChangeMask      = theMsgR.mChangeMask;
		mNewName         = theMsgR.mNewName;
		mNewDisplayName  = theMsgR.mNewDisplayName;
		mNewVersion      = theMsgR.mNewVersion;
		mNewProtoName    = theMsgR.mNewProtoName;
		mNewProtoVersion = theMsgR.mNewProtoVersion;
		mNewNetAddress   = theMsgR.mNewNetAddress;
		mNewLifespan     = theMsgR.mNewLifespan;

		SetNewBlob(theMsgR.mNewBlob, theMsgR.mNewBlobLen);
	}

	return *this;
}


// TMsgDirChangeService::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirChangeService::Pack(void)
{
	WTRACE("TMsgDirChangeService::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirChangeService);
	TMsgDirServiceBase::Pack();

	WDBG_LL("TMsgDirChangeService::Pack Appending message data");
	AppendByte(mChangeMask);
	Append_PW_STRING(mNewName);
	Append_PW_STRING(mNewDisplayName);
	Append_PW_STRING(mNewVersion);
	Append_PW_STRING(mNewProtoName);
	Append_PW_STRING(mNewProtoVersion);
	Append_PW_STRING(mNewNetAddress);
	AppendLong(mNewLifespan);
	AppendShort(mNewBlobLen);
	if (mNewBlobLen > 0) AppendBytes(mNewBlobLen, mNewBlob);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirChangeService::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirChangeService::Unpack(void)
{
	WTRACE("TMsgDirChangeService::Unpack");
	TMsgDirServiceBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirChangeService))
	{
		WDBG_AH("TMsgDirChangeService::Unpack Not a DirChangeService message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirChangeService message.");
	}

	WDBG_LL("TMsgDirChangeService::Unpack Reading message data");
	mChangeMask = ReadByte();
	ReadWString(mNewName);
	ReadWString(mNewDisplayName);
	ReadWString(mNewVersion);
	ReadWString(mNewProtoName);
	ReadWString(mNewProtoVersion);
	ReadWString(mNewNetAddress);
	mNewLifespan = ReadLong();
	mNewBlobLen  = ReadShort();

	SetNewBlob(ReadBytes(mNewBlobLen), mNewBlobLen);

	// Unpack peer data if needed
	UnpackPeerData();
}


void
TMsgDirChangeService::SetNewBlob(const void* theBlob, unsigned short theLen)
{
	delete mNewBlob;  mNewBlob = NULL;
	mNewBlobLen = theLen;

	if (mNewBlobLen > 0)
	{
		mNewBlob = new unsigned char [mNewBlobLen];
		memcpy(mNewBlob, theBlob, mNewBlobLen);
	}
}
