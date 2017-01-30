// TMsgDirServiceBase.h

// Base class for directory server entry messages.  This class is not a
// directory server message itself.  It is just further refinement of
// TMessage for use in directory messages.


#include "common/won.h"
#include "msg/TMessage.h"
#include "TMsgDirPeerDataBase.h"
#include "TMsgDirServiceBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirPeerDataBase;
	using WONMsg::TMsgDirServiceBase;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirServiceBase::TMsgDirServiceBase(void) :
	TMsgDirPeerDataBase(),
	mDirectoryPath(),
	mName(),
	mDisplayName(),
	mVersion(),
	mProtoName(),
	mProtoVersion(),
	mNetAddress()
{}


// TMessage ctor
TMsgDirServiceBase::TMsgDirServiceBase(const TMessage& theMsgR) :
	TMsgDirPeerDataBase(theMsgR),
	mDirectoryPath(),
	mName(),
	mDisplayName(),
	mVersion(),
	mProtoName(),
	mProtoVersion(),
	mNetAddress()
{}


// Copy ctor
TMsgDirServiceBase::TMsgDirServiceBase(const TMsgDirServiceBase& theMsgR) :
	TMsgDirPeerDataBase(theMsgR),
	mDirectoryPath(theMsgR.mDirectoryPath),
	mName(theMsgR.mName),
	mDisplayName(theMsgR.mDisplayName),
	mVersion(theMsgR.mVersion),
	mProtoName(theMsgR.mProtoName),
	mProtoVersion(theMsgR.mProtoVersion),
	mNetAddress(theMsgR.mNetAddress)
{}


// Destructor
TMsgDirServiceBase::~TMsgDirServiceBase(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirServiceBase&
TMsgDirServiceBase::operator=(const TMsgDirServiceBase& theMsgR)
{
	TMsgDirPeerDataBase::operator=(theMsgR);
	mDirectoryPath = theMsgR.mDirectoryPath;
	mName          = theMsgR.mName;
	mDisplayName   = theMsgR.mDisplayName;
	mVersion       = theMsgR.mVersion;
	mProtoName     = theMsgR.mProtoName;
	mProtoVersion  = theMsgR.mProtoVersion;
	mNetAddress    = theMsgR.mNetAddress;

	return *this;
}


// TMsgDirServiceBase::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirServiceBase::Pack(void)
{
	WTRACE("TMsgDirServiceBase::Pack");
	TMsgDirPeerDataBase::Pack();

	WDBG_LL("TMsgDirServiceBase::Pack Appending message data");
	Append_PW_STRING(mDirectoryPath);
	Append_PW_STRING(mName);
	Append_PW_STRING(mDisplayName);
	Append_PW_STRING(mVersion);
	Append_PW_STRING(mProtoName);
	Append_PW_STRING(mProtoVersion);
	Append_PW_STRING(mNetAddress);

	return GetDataPtr();
}


// TMsgDirServiceBase::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirServiceBase::Unpack(void)
{
	WTRACE("TMsgDirServiceBase::Unpack");
	TMsgDirPeerDataBase::Unpack();

	WDBG_LL("TMsgDirServiceBase::Unpack Reading message data");
	ReadWString(mDirectoryPath);
	ReadWString(mName);
	ReadWString(mDisplayName);
	ReadWString(mVersion);
	ReadWString(mProtoName);
	ReadWString(mProtoVersion);
	ReadWString(mNetAddress);
}
