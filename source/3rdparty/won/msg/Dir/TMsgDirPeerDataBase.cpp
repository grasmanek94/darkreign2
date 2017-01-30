// TMsgDirPeerDataBase.h

// Base class for directory server messages that contain peer data.  This
// class is not a directory server message itself.  It provides methods to
// pack and unpack a PeerData if needed.

// Note that this message does not implement Pack and Unpack as PeerData is
// always appended to the end of DirServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "TMsgDirPeerDataBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirPeerDataBase;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirPeerDataBase::TMsgDirPeerDataBase(void) :
	TMessage(),
	mPeerKey(),
	mPeerIndex(0)
{}


// TMessage ctor
TMsgDirPeerDataBase::TMsgDirPeerDataBase(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mPeerKey(),
	mPeerIndex(0)
{}


// Copy ctor
TMsgDirPeerDataBase::TMsgDirPeerDataBase(const TMsgDirPeerDataBase& theMsgR) :
	TMessage(theMsgR),
	mPeerKey(theMsgR.mPeerKey),
	mPeerIndex(theMsgR.mPeerIndex)
{}


// Destructor
TMsgDirPeerDataBase::~TMsgDirPeerDataBase(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirPeerDataBase&
TMsgDirPeerDataBase::operator=(const TMsgDirPeerDataBase& theMsgR)
{
	TMessage::operator=(theMsgR);
	mPeerKey   = theMsgR.mPeerKey;
	mPeerIndex = theMsgR.mPeerIndex;
	return *this;
}


// TMsgDirPeerDataBase::PackPeerData
// Hook to pack the peer data if needed.  Only appends peer data if peer
// data is defined.
void
TMsgDirPeerDataBase::PackPeerData(void)
{
	WTRACE("TMsgDirPeerDataBase::PackPeerData");
	if (mPeerKey.size() > 0)
	{
		WDBG_LL("TMsgDirPeerDataBase::PackPeerData Appending peer data.");
		Append_PA_STRING(mPeerKey);
		AppendLong(mPeerIndex);
	}
	else
		WDBG_LL("TMsgDirPeerDataBase::PackPeerData No peer data to append.");
}


// TMsgDirPeerDataBase::UnpackPeerData
// Hook to unpack the peer data if needed.  Only reads peer data if peer
// data is present.
void
TMsgDirPeerDataBase::UnpackPeerData(void)
{
	WTRACE("TMsgDirPeerDataBase::UnpackPeerData");
	if (BytesLeftToRead() > 0)
	{
		WDBG_LL("TMsgDirPeerDataBase::UnpackPeerData Reading peer data.");
		ReadString(mPeerKey);
		mPeerIndex = ReadLong();
	}
	else
	{
		WDBG_LL("TMsgDirPeerDataBase::UnpackPeerData No peer data to read.");
		mPeerKey   = string();
		mPeerIndex = 0;
	}
}
