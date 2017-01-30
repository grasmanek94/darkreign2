// TMsgDirAddService.h

// DirectoryServer Add Entry message.  Defines an entry to add.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "DirServerSKMasks.h"
#include "TMsgDirServiceBase.h"
#include "TMsgDirAddService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirServiceBase;
	using WONMsg::TMsgDirAddService;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirAddService::TMsgDirAddService(void) :
	TMsgDirServiceBase(),
	mLifespan(0),
	mAddMask(WONMsg::SKMASK_ALL),
	mBlobLen(0),
	mBlob(NULL)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirAddService);
}


// TMessage ctor
TMsgDirAddService::TMsgDirAddService(const TMessage& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mLifespan(0),
	mAddMask(WONMsg::SKMASK_ALL),
	mBlobLen(0),
	mBlob(NULL)
{
	Unpack();
}


// Copy ctor
TMsgDirAddService::TMsgDirAddService(const TMsgDirAddService& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mLifespan(theMsgR.mLifespan),
	mAddMask(theMsgR.mAddMask),
	mBlobLen(0),
	mBlob(NULL)
{
	SetBlob(theMsgR.mBlob, theMsgR.mBlobLen);
}


// Destructor
TMsgDirAddService::~TMsgDirAddService(void)
{
	delete mBlob;
}


// ** Public Methods

// Assignment operator
TMsgDirAddService&
TMsgDirAddService::operator=(const TMsgDirAddService& theMsgR)
{
	if (this != &theMsgR)  // protect vs a = a
	{
		TMsgDirServiceBase::operator=(theMsgR);
		mLifespan = theMsgR.mLifespan;
		mAddMask  = theMsgR.mAddMask;

		SetBlob(theMsgR.mBlob, theMsgR.mBlobLen);
	}

	return *this;
}


// TMsgDirAddService::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirAddService::Pack(void)
{
	WTRACE("TMsgDirAddService::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirAddService);
	TMsgDirServiceBase::Pack();

	WDBG_LL("TMsgDirAddService::Pack Appending message data");
	AppendLong(mLifespan);
	AppendByte(mAddMask);
	AppendShort(mBlobLen);
	if (mBlobLen > 0) AppendBytes(mBlobLen, mBlob);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirAddService::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirAddService::Unpack(void)
{
	WTRACE("TMsgDirAddService::Unpack");
	TMsgDirServiceBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirAddService))
	{
		WDBG_AH("TMsgDirAddService::Unpack Not a DirAddService message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirAddService message.");
	}

	WDBG_LL("TMsgDirAddService::Unpack Reading message data");
	delete mBlob;  mBlob = NULL;
	mLifespan = ReadLong();
	mAddMask  = ReadByte();
	mBlobLen  = ReadShort();

	SetBlob(ReadBytes(mBlobLen), mBlobLen);

	// Unpack peer data if needed
	UnpackPeerData();
}


void
TMsgDirAddService::SetBlob(const void* theBlob, unsigned short theLen)
{
	delete mBlob;  mBlob = NULL;
	mBlobLen = theLen;

	if (theLen > 0)
	{
		mBlob = new unsigned char [mBlobLen];
		memcpy(mBlob, theBlob, mBlobLen);
	}
}
