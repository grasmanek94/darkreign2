// MMsgCommIsUserPresent.cpp

// Common Ping message classes.  Supports ping requests and replys to
// WON servers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesComm.h"
#include "MMsgCommIsUserPresent.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommIsUserPresent;
	using WONMsg::MMsgCommIsUserPresentReply;
};


// ** MMsgCommIsUserPresent **

// ** Constructors / Destructor

// Default ctor
MMsgCommIsUserPresent::MMsgCommIsUserPresent(void) :
	MiniMessage(),
	mTag(0),
	mShouldSendNack(true),
	mCaseInsensitive(false),
	mUserName()
{
	WTRACE("MMsgCommIsUserPresent::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommIsUserPresent);
}


// MiniMessage ctor
MMsgCommIsUserPresent::MMsgCommIsUserPresent(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommIsUserPresent::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommIsUserPresent::MMsgCommIsUserPresent(const MMsgCommIsUserPresent& theMsgR) :
	MiniMessage(theMsgR),
	mTag(theMsgR.mTag),
	mShouldSendNack(theMsgR.mShouldSendNack),
	mCaseInsensitive(theMsgR.mCaseInsensitive),
	mUserName(theMsgR.mUserName)
{
	WTRACE("MMsgCommIsUserPresent::ctor(copy)");
}


// Destructor
MMsgCommIsUserPresent::~MMsgCommIsUserPresent(void)
{
	WTRACE("MMsgCommIsUserPresent::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommIsUserPresent&
MMsgCommIsUserPresent::operator=(const MMsgCommIsUserPresent& theMsgR)
{
	WTRACE("MMsgCommIsUserPresent::operator=");
	MiniMessage::operator=(theMsgR);

	mTag             = theMsgR.mTag;
	mShouldSendNack  = theMsgR.mShouldSendNack;
	mCaseInsensitive = theMsgR.mCaseInsensitive;
	mUserName        = theMsgR.mUserName;

	return *this;
}


// MMsgCommIsUserPresent::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommIsUserPresent::Pack(void)
{
	WTRACE("MMsgCommIsUserPresent::Pack");

	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommIsUserPresent);
	MiniMessage::Pack();

	// Generate and pack in startTick
	WDBG_LL("MMsgCommIsUserPresent::Pack Appending message data");
	AppendShort(mTag);

	BYTE b = 0;
	
	if (mShouldSendNack)
		b |= cShouldSendNackBit;
	if (mCaseInsensitive)
		b |= cCaseInsensitiveBit;

	AppendByte(b);
	Append_PW_STRING(mUserName);

	return GetDataPtr();
}


// MMsgCommIsUserPresent::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommIsUserPresent::Unpack(void)
{
	WTRACE("MMsgCommIsUserPresent::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniCommonService ||
	    GetMessageType() != WONMsg::MiniCommIsUserPresent)
	{
		WDBG_AH("MMsgCommIsUserPresent::Unpack Not a CommIsUserPresent message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommIsUserPresent message.");
	}

	// Read attributes
	WDBG_LL("MMsgCommIsUserPresent::Unpack Reading message data");
	mTag            = ReadShort();
	BYTE b = ReadByte();
	mShouldSendNack = (b & cShouldSendNackBit) != 0;
	mCaseInsensitive= (b & cCaseInsensitiveBit) != 0;
	mUserName       = Read_PW_STRING();
}


// ** MMsgCommIsUserPresentReply **

// ** Constructors / Destructor

// Default ctor
MMsgCommIsUserPresentReply::MMsgCommIsUserPresentReply(void) :
	MiniMessage(),
	mTag(0),
	mIsUserPresent(false)
{
	WTRACE("MMsgCommIsUserPresentReply::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommIsUserPresentReply);
}


// MiniMessage ctor
MMsgCommIsUserPresentReply::MMsgCommIsUserPresentReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommIsUserPresentReply::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommIsUserPresentReply::MMsgCommIsUserPresentReply(const MMsgCommIsUserPresentReply& theMsgR) :
	MiniMessage(theMsgR),
	mTag(theMsgR.mTag),
	mIsUserPresent(theMsgR.mIsUserPresent)
{
	WTRACE("MMsgCommIsUserPresentReply::ctor(copy)");
}


// Destructor
MMsgCommIsUserPresentReply::~MMsgCommIsUserPresentReply(void)
{
	WTRACE("MMsgCommIsUserPresentReply::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommIsUserPresentReply&
MMsgCommIsUserPresentReply::operator=(const MMsgCommIsUserPresentReply& theMsgR)
{
	WTRACE("MMsgCommIsUserPresent::operator=");
	MiniMessage::operator=(theMsgR);

	mTag           = theMsgR.mTag;
	mIsUserPresent = theMsgR.mIsUserPresent;

	return *this;
}


// MMsgCommIsUserPresentReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommIsUserPresentReply::Pack(void)
{
	WTRACE("MMsgCommIsUserPresentReply::Pack");

	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommIsUserPresentReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgCommIsUserPresentReply::Pack Appending message data");
	AppendShort(mTag);
	AppendByte(mIsUserPresent);

	return GetDataPtr();
}


// MMsgCommIsUserPresentReply::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommIsUserPresentReply::Unpack(void)
{
	WTRACE("MMsgCommIsUserPresentReply::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniCommonService ||
	    GetMessageType() != WONMsg::MiniCommIsUserPresentReply)
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a CommIsUserPresentReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommIsUserPresentReply message.");
	}

	WDBG_LL("MMsgCommIsUserPresentReply::Unpack Reading message data");

	mTag           = ReadShort();
	mIsUserPresent = (ReadByte() != 0);
}
