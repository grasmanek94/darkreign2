// MMsgCommGetNumUsers.cpp

// Common Ping message classes.  Supports ping requests and replys to
// WON servers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesComm.h"
#include "MMsgCommGetNumUsers.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommGetNumUsers;
	using WONMsg::MMsgCommGetNumUsersReply;
};


// ** MMsgCommGetNumUsers **

// ** Constructors / Destructor

// Default ctor
MMsgCommGetNumUsers::MMsgCommGetNumUsers(void) :
	MiniMessage(),
	mTag(0)
{
	WTRACE("MMsgCommGetNumUsers::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNumUsers);
}


// MiniMessage ctor
MMsgCommGetNumUsers::MMsgCommGetNumUsers(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommGetNumUsers::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommGetNumUsers::MMsgCommGetNumUsers(const MMsgCommGetNumUsers& theMsgR) :
	MiniMessage(theMsgR),
	mTag(theMsgR.mTag)
{
	WTRACE("MMsgCommGetNumUsers::ctor(copy)");
}


// Destructor
MMsgCommGetNumUsers::~MMsgCommGetNumUsers(void)
{
	WTRACE("MMsgCommGetNumUsers::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommGetNumUsers&
MMsgCommGetNumUsers::operator=(const MMsgCommGetNumUsers& theMsgR)
{
	WTRACE("MMsgCommGetNumUsers::operator=");
	MiniMessage::operator=(theMsgR);
	mTag = theMsgR.mTag;
	return *this;
}


// MMsgCommGetNumUsers::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommGetNumUsers::Pack(void)
{
	WTRACE("MMsgCommGetNumUsers::Pack");

	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNumUsers);
	MiniMessage::Pack();

	// Generate and pack in startTick
	WDBG_LL("MMsgCommGetNumUsers::Pack Appending message data");
	AppendShort(mTag);

	return GetDataPtr();
}


// MMsgCommGetNumUsers::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommGetNumUsers::Unpack(void)
{
	WTRACE("MMsgCommGetNumUsers::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniCommonService ||
		GetMessageType() != WONMsg::MiniCommGetNumUsers)
	{
		WDBG_AH("MMsgCommGetNumUsers::Unpack Not a CommGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommGetNumUsers message.");
	}

	// Read attributes
	WDBG_LL("MMsgCommGetNumUsers::Unpack Reading message data");
	mTag = ReadShort();
}


// ** MMsgCommGetNumUsersReply **

// ** Constructors / Destructor

// Default ctor
MMsgCommGetNumUsersReply::MMsgCommGetNumUsersReply(void) :
	MiniMessage(),
	mTag(0),
	mNumActiveUsers(0),
	mUserCapacity(0)
{
	WTRACE("MMsgCommGetNumUsersReply::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNumUsersReply);
}


// MiniMessage ctor
MMsgCommGetNumUsersReply::MMsgCommGetNumUsersReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommGetNumUsersReply::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommGetNumUsersReply::MMsgCommGetNumUsersReply(const MMsgCommGetNumUsersReply& theMsgR) :
	MiniMessage(theMsgR),
	mTag(theMsgR.mTag),
	mNumActiveUsers(theMsgR.mNumActiveUsers),
	mUserCapacity(theMsgR.mUserCapacity)
{
	WTRACE("MMsgCommGetNumUsersReply::ctor(copy)");
}


// Destructor
MMsgCommGetNumUsersReply::~MMsgCommGetNumUsersReply(void)
{
	WTRACE("MMsgCommGetNumUsersReply::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommGetNumUsersReply&
MMsgCommGetNumUsersReply::operator=(const MMsgCommGetNumUsersReply& theMsgR)
{
	WTRACE("MMsgCommGetNumUsers::operator=");
	MiniMessage::operator=(theMsgR);

	mTag            = theMsgR.mTag;
	mNumActiveUsers = theMsgR.mNumActiveUsers;
	mUserCapacity   = theMsgR.mUserCapacity;

	return *this;
}


// MMsgCommGetNumUsersReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommGetNumUsersReply::Pack(void)
{
	WTRACE("MMsgCommGetNumUsersReply::Pack");

	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNumUsersReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgCommGetNumUsersReply::Pack Appending message data");
	AppendShort(mTag);
	AppendShort(mNumActiveUsers);
	AppendShort(mUserCapacity);

	return GetDataPtr();
}


// MMsgCommGetNumUsersReply::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommGetNumUsersReply::Unpack(void)
{
	WTRACE("MMsgCommGetNumUsersReply::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniCommonService ||
	    GetMessageType() != WONMsg::MiniCommGetNumUsersReply)
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a CommGetNumUsersReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommGetNumUsersReply message.");
	}

	WDBG_LL("MMsgCommGetNumUsersReply::Unpack Reading message data");

	mTag            = ReadShort();
	mNumActiveUsers = ReadShort();
	mUserCapacity   = ReadShort();
}
