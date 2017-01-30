// TMsgAuth1GetPubKeys.h

// AuthServer request to fetch list of public keys for a given Auth Familty 1.
// Requests the AuthServer to return its current public key block for a family.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1GetPubKeys.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::TMsgAuthRawBufferBase;
	using WONMsg::TMsgAuth1GetPubKeys;
	using WONMsg::TMsgAuth1GetPubKeysReply;
};


// ** TMsgAuth1GetPubKeys **

// ** Constructors / Destructor

// Default ctor
TMsgAuth1GetPubKeys::TMsgAuth1GetPubKeys(WONMsg::ServiceType theServType) :
	TMessage()
{
	SetServiceType(theServType);
	SetMessageType(WONMsg::Auth1GetPubKeys);
}


// TMessage ctor
TMsgAuth1GetPubKeys::TMsgAuth1GetPubKeys(const TMessage& theMsgR) :
	TMessage(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1GetPubKeys::TMsgAuth1GetPubKeys(const TMsgAuth1GetPubKeys& theMsgR) :
	TMessage(theMsgR)
{}


// Destructor
TMsgAuth1GetPubKeys::~TMsgAuth1GetPubKeys()
{}


// ** Public Methods

// Assignment operator (incomplete)
TMsgAuth1GetPubKeys&
TMsgAuth1GetPubKeys::operator=(const TMsgAuth1GetPubKeys& theMsgR)
{
	TMessage::operator=(theMsgR);
	return *this;
}


// TMsgAuth1GetPubKeys::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1GetPubKeys::Pack(void)
{
	WTRACE("TMsgAuth1GetPubKeys::Pack");
//	SetMessageType(WONMsg::Auth1GetPubKeys);
	return TMessage::Pack();
}


// TMsgAuth1GetPubKeys::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1GetPubKeys::Unpack(void)
{
	WTRACE("TMsgAuth1GetPubKeys::Unpack");
	TMessage::Unpack();

	if ( GetMessageType() != WONMsg::Auth1GetPubKeys
	 &&  GetMessageType() != WONMsg::Auth1GetPubKeys23 )
	{
		WDBG_AH("TMsgAuth1GetPubKeys::Unpack Not a Auth1GetPubKeys message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1GetPubKeys message.");
	}
}


// ** TMsgAuth1GetPubKeysReply **

// ** Constructors / Destructor

// Default ctor
TMsgAuth1GetPubKeysReply::TMsgAuth1GetPubKeysReply(WONMsg::ServiceType theServType) :
	TMsgAuthRawBufferBase(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(theServType);
	SetMessageType(WONMsg::Auth1GetPubKeysReply);
}


// TMessage ctor
TMsgAuth1GetPubKeysReply::TMsgAuth1GetPubKeysReply(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy ctor
TMsgAuth1GetPubKeysReply::TMsgAuth1GetPubKeysReply(const TMsgAuth1GetPubKeysReply& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mStatus(theMsgR.mStatus)
{}


// Destructor
TMsgAuth1GetPubKeysReply::~TMsgAuth1GetPubKeysReply(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1GetPubKeysReply&
TMsgAuth1GetPubKeysReply::operator=(const TMsgAuth1GetPubKeysReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		mStatus = theMsgR.mStatus;
	}
	return *this;
}


// TMsgAuth1GetPubKeysReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1GetPubKeysReply::Pack(void)
{
	WTRACE("TMsgAuth1GetPubKeysReply::Pack");
//	SetMessageType(WONMsg::Auth1GetPubKeysReply);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1GetPubKeysReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1GetPubKeysReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1GetPubKeysReply::Unpack(void)
{
	WTRACE("TMsgAuth1GetPubKeysReply::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if (GetMessageType() != WONMsg::Auth1GetPubKeysReply
	 && GetMessageType() != WONMsg::Auth1GetPubKeysReply23)
	{
		WDBG_AH("TMsgAuth1GetPubKeysReply::Unpack Not a Auth1GetPubKeysReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1GetPubKeysReply message.");
	}

	WDBG_LL("TMsgAuth1GetPubKeysReply::Unpack Reading message data");
	mStatus = static_cast<ServerStatus>((short)ReadShort());
	UnpackRawBuf();
}
