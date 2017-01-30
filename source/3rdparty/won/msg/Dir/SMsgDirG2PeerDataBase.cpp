// SMsgDirG2PeerDataBase

// Base class for directory server messages that contain peer data.  This
// class is not a directory server message itself.  It provides methods to
// pack and unpack a PeerData if needed.

// Note that this message does not implement Pack and Unpack as PeerData is
// always appended to the end of DirServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "SMsgDirG2PeerDataBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2PeerDataBase;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2PeerDataBase::SMsgDirG2PeerDataBase(void) :
	SmallMessage(),
	mPeerKey(),
	mPeerIndex(0),
	mPeerUser(0)
{}


// SmallMessage ctor
SMsgDirG2PeerDataBase::SMsgDirG2PeerDataBase(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mPeerKey(),
	mPeerIndex(0),
	mPeerUser(0)
{}


// Copy ctor
SMsgDirG2PeerDataBase::SMsgDirG2PeerDataBase(const SMsgDirG2PeerDataBase& theMsgR) :
	SmallMessage(theMsgR),
	mPeerKey(theMsgR.mPeerKey),
	mPeerIndex(theMsgR.mPeerIndex),
	mPeerUser(theMsgR.mPeerUser)
{}


// Destructor
SMsgDirG2PeerDataBase::~SMsgDirG2PeerDataBase(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerDataBase&
SMsgDirG2PeerDataBase::operator=(const SMsgDirG2PeerDataBase& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mPeerKey   = theMsgR.mPeerKey;
	mPeerIndex = theMsgR.mPeerIndex;
	mPeerUser  = theMsgR.mPeerUser;
	return *this;
}


// SMsgDirG2PeerDataBase::PackPeerData
// Hook to pack the peer data if needed.  Only appends peer data if peer
// data is defined.
void
SMsgDirG2PeerDataBase::PackPeerData(void)
{
	WTRACE("SMsgDirG2PeerDataBase::PackPeerData");
	if (mPeerKey.size() > 0)
	{
		WDBG_LL("SMsgDirG2PeerDataBase::PackPeerData Appending peer data.");
		Append_PA_STRING(mPeerKey);
		AppendLong(mPeerIndex);
		AppendLong(mPeerUser);
	}
	else
		WDBG_LL("SMsgDirG2PeerDataBase::PackPeerData No peer data to append.");
}


// SMsgDirG2PeerDataBase::UnpackPeerData
// Hook to unpack the peer data if needed.  Only reads peer data if peer
// data is present.  Note that mPeerUser was added later and may or may not be
// present.
void
SMsgDirG2PeerDataBase::UnpackPeerData(void)
{
	WTRACE("SMsgDirG2PeerDataBase::UnpackPeerData");
	if (BytesLeftToRead() > 0)
	{
		WDBG_LL("SMsgDirG2PeerDataBase::UnpackPeerData Reading peer data.");
		ReadString(mPeerKey);
		mPeerIndex = ReadLong();
		if (BytesLeftToRead() > 0)
		{
			WDBG_LL("SMsgDirG2PeerDataBase::UnpackPeerData Reading user id.");
			mPeerUser = ReadLong();
		}
		else
			mPeerUser = 0;
	}
	else
	{
		WDBG_LL("SMsgDirG2PeerDataBase::UnpackPeerData No peer data to read.");
		mPeerKey   = string();
		mPeerIndex = 0;
		mPeerUser  = 0;
	}
}
