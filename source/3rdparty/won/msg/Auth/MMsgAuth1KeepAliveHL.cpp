#include "common/WON.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
/*  obsolete
#include "msg\MMsgAuth1KeepAliveHL.h"

namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgAuth1KeepAliveHL;
	using WONMsg::MMsgAuth1KeepAliveReplyHL;
};

MMsgAuth1KeepAliveHL::MMsgAuth1KeepAliveHL(void) : MiniMessage()
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1KeepAliveHL);
}

MMsgAuth1KeepAliveHL::~MMsgAuth1KeepAliveHL(void)
{}

MMsgAuth1KeepAliveHL::MMsgAuth1KeepAliveHL(const MMsgAuth1KeepAliveHL& theMsgR) :
    MiniMessage(theMsgR),
	mUserId(theMsgR.mUserId),
    mRawBuf(theMsgR.mRawBuf)
{}

MMsgAuth1KeepAliveHL::MMsgAuth1KeepAliveHL(const MiniMessage& theMsgR) : 
    MiniMessage(theMsgR)
{
	Unpack();
}

MMsgAuth1KeepAliveHL& MMsgAuth1KeepAliveHL::operator =(const MMsgAuth1KeepAliveHL& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    mUserId = theMsgR.mUserId;
    mRawBuf = theMsgR.mRawBuf;

	return *this;
}

void* MMsgAuth1KeepAliveHL::Pack(void)
{
	WTRACE("MMsgAuth1KeepAliveHL::Pack");

	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1KeepAliveHL);
	MiniMessage::Pack();

	WDBG_LL("MMsgAuth1KeepAliveHL::Pack Appending message data");

	// append username
	AppendLong(mUserId);
	AppendBytes( mRawBuf.size(), mRawBuf.data() );

	return GetDataPtr();
}

void MMsgAuth1KeepAliveHL::Unpack(void)
{
	WTRACE("MMsgAuth1KeepAliveHL::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::Auth1LoginHL || 
        GetMessageType() != WONMsg::Auth1KeepAliveHL)
	{
		WDBG_AH("MMsgAuth1KeepAliveHL::Unpack Not a RoutingBanUser message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingBanUser message.");
	}

	WDBG_LL("MMsgAuth1KeepAliveHL::Unpack Reading message data");

	// read in userid
	mUserId = ReadLong();
    mRawBuf.assign( (const unsigned char*)ReadBytes( BytesLeftToRead() ), BytesLeftToRead() );
}

///////////////
MMsgAuth1KeepAliveReplyHL::MMsgAuth1KeepAliveReplyHL(void) : MiniMessage()
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1KeepAliveReplyHL);
}

MMsgAuth1KeepAliveReplyHL::~MMsgAuth1KeepAliveReplyHL(void)
{}

MMsgAuth1KeepAliveReplyHL::MMsgAuth1KeepAliveReplyHL(const MMsgAuth1KeepAliveReplyHL& theMsgR) :
    MiniMessage(theMsgR),
	mRawBuf(theMsgR.mRawBuf)
{}

MMsgAuth1KeepAliveReplyHL::MMsgAuth1KeepAliveReplyHL(const MiniMessage& theMsgR) : 
    MiniMessage(theMsgR)
{
	Unpack();
}

MMsgAuth1KeepAliveReplyHL& MMsgAuth1KeepAliveReplyHL::operator =(const MMsgAuth1KeepAliveReplyHL& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    mRawBuf = theMsgR.mRawBuf;

	return *this;
}

void* MMsgAuth1KeepAliveReplyHL::Pack(void)
{
	WTRACE("MMsgAuth1KeepAliveReplyHL::Pack");

	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1KeepAliveReplyHL);
	MiniMessage::Pack();

	WDBG_LL("MMsgAuth1KeepAliveReplyHL::Pack Appending message data");

	AppendBytes( mRawBuf.size(), mRawBuf.data() );

	return GetDataPtr();
}

void MMsgAuth1KeepAliveReplyHL::Unpack(void)
{
	WTRACE("MMsgAuth1KeepAliveReplyHL::Unpack");
	MiniMessage::Unpack();

	if (GetServiceType() != WONMsg::Auth1LoginHL || 
        GetMessageType() != WONMsg::Auth1KeepAliveReplyHL)
	{
		WDBG_AH("MMsgAuth1KeepAliveReplyHL::Unpack Not a RoutingBanUser message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingBanUser message.");
	}

	WDBG_LL("MMsgAuth1KeepAliveReplyHL::Unpack Reading message data");

	// read in username
	mRawBuf.assign( (unsigned char *)ReadBytes( BytesLeftToRead() ), BytesLeftToRead() );
}
*/