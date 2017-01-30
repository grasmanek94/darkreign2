#ifdef DELETED
// TMsgAuth1Login.h

// AuthServer messages implementing the Auth1 Family login protocol.
// This header/source implements messages:
//	Auth1LoginRequest
//	Auth1NewLoginRequest
// It also includes the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequest begins login for an existing user and is sent from the Client to
// the AuthServer.

// Auth1NewLoginRequest begins login for a new user and is sent from the Client to
// the AuthServer.

// Auth1LoginReply is sent in response to both Auth1LoginRequest and
// Auth1NewLoginRequest from the AuthServer to the client and completes the login.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1Login.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgAuthRawBufferBase;
	using WONMsg::TMsgAuth1LoginBase;
	using WONMsg::TMsgAuth1LoginRequest;
	using WONMsg::TMsgAuth1NewLoginRequest;
};


// ** TMsgAuth1LoginBase **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginBase::TMsgAuth1LoginBase() :
	TMsgAuthRawBufferBase(),
	mKeyBlockId(0),
	mNeedKey(false),
	mLoginName(),
	mCommunityName(),
	mNicknameKey()
{
}


// TMessage ctor
TMsgAuth1LoginBase::TMsgAuth1LoginBase(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mKeyBlockId(0),
	mNeedKey(false),
	mLoginName(),
	mCommunityName(),
	mNicknameKey()
{}


// Copy ctor
TMsgAuth1LoginBase::TMsgAuth1LoginBase(const TMsgAuth1LoginBase& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mKeyBlockId(theMsgR.mKeyBlockId),
	mNeedKey(theMsgR.mNeedKey),
	mLoginName(theMsgR.mLoginName),
	mCommunityName(theMsgR.mCommunityName),
	mNicknameKey(theMsgR.mNicknameKey)
{}


// Destructor
TMsgAuth1LoginBase::~TMsgAuth1LoginBase(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginBase&
TMsgAuth1LoginBase::operator=(const TMsgAuth1LoginBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		mKeyBlockId    = theMsgR.mKeyBlockId;
		mNeedKey       = theMsgR.mNeedKey;
		mLoginName     = theMsgR.mLoginName;
		mCommunityName = theMsgR.mCommunityName;
		mNicknameKey   = theMsgR.mNicknameKey;
	}
	return *this;
}


// TMsgAuth1LoginBase::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginBase::Pack(void)
{
	WTRACE("TMsgAuth1LoginBase::Pack");
//	SetServiceType(WONMsg::Auth1Login);
//	SetMessageType(WONMsg::Auth1LoginRequest);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1LoginBase::Pack Appending message data");
	AppendShort(mKeyBlockId);
	AppendByte(mNeedKey ? 1 : 0);
	Append_PW_STRING(mLoginName);
	Append_PW_STRING(mCommunityName);
	Append_PW_STRING(mNicknameKey);

	return GetDataPtr();
}


// TMsgAuth1LoginBase::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1LoginBase::Unpack(void)
{
	WTRACE("TMsgAuth1LoginBase::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	WDBG_LL("TMsgAuth1LoginBase::Unpack Reading message data");
	mKeyBlockId = ReadShort();
	mNeedKey    = (ReadByte() ? true : false);
	ReadWString(mLoginName);
	ReadWString(mCommunityName);
	ReadWString(mNicknameKey);
}


// ** TMsgAuth1LoginRequest **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginRequest::TMsgAuth1LoginRequest(void) :
	TMsgAuth1LoginBase()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginRequest);
}


// TMessage ctor
TMsgAuth1LoginRequest::TMsgAuth1LoginRequest(const TMessage& theMsgR) :
	TMsgAuth1LoginBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1LoginRequest::TMsgAuth1LoginRequest(const TMsgAuth1LoginRequest& theMsgR) :
	TMsgAuth1LoginBase(theMsgR)
{}


// Destructor
TMsgAuth1LoginRequest::~TMsgAuth1LoginRequest(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginRequest&
TMsgAuth1LoginRequest::operator=(const TMsgAuth1LoginRequest& theMsgR)
{
	TMsgAuth1LoginBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1LoginRequest::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginRequest::Pack(void)
{
	WTRACE("TMsgAuth1LoginRequest::Pack");
//	SetServiceType(WONMsg::Auth1Login);
//	SetMessageType(WONMsg::Auth1LoginRequest);
	TMsgAuth1LoginBase::Pack();

	WDBG_LL("TMsgAuth1LoginRequest::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1LoginRequest::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1LoginRequest::Unpack(void)
{
	WTRACE("TMsgAuth1LoginRequest::Unpack");
	TMsgAuth1LoginBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	  (  GetMessageType() != WONMsg::Auth1LoginRequest
	  && GetMessageType() != WONMsg::Auth1LoginRequest23))
	{
		WDBG_AH("TMsgAuth1LoginBase::Unpack Not a Auth1LoginRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginRequest message.");
	}

	WDBG_LL("TMsgAuth1LoginBase::Unpack Reading message data");
	UnpackRawBuf();
}


// ** TMsgAuth1NewLoginRequest **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1NewLoginRequest::TMsgAuth1NewLoginRequest(void) :
	TMsgAuth1LoginBase(),
	mNickname()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1NewLoginRequest);
}


// TMessage ctor
TMsgAuth1NewLoginRequest::TMsgAuth1NewLoginRequest(const TMessage& theMsgR) :
	TMsgAuth1LoginBase(theMsgR),
	mNickname()
{
	Unpack();
}


// Copy ctor
TMsgAuth1NewLoginRequest::TMsgAuth1NewLoginRequest(const TMsgAuth1NewLoginRequest& theMsgR) :
	TMsgAuth1LoginBase(theMsgR),
	mNickname(theMsgR.mNickname)
{}


// Destructor
TMsgAuth1NewLoginRequest::~TMsgAuth1NewLoginRequest(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1NewLoginRequest&
TMsgAuth1NewLoginRequest::operator=(const TMsgAuth1NewLoginRequest& theMsgR)
{
	TMsgAuth1LoginBase::operator=(theMsgR);
	mNickname = theMsgR.mNickname;
	return *this;
}


// TMsgAuth1NewLoginRequest::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1NewLoginRequest::Pack(void)
{
	WTRACE("TMsgAuth1NewLoginRequest::Pack");
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1NewLoginRequest);
	TMsgAuth1LoginBase::Pack();

	WDBG_LL("TMsgAuth1NewLoginRequest::Pack Appending message data");
	Append_PW_STRING(mNickname);
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1NewLoginRequest::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1NewLoginRequest::Unpack(void)
{
	WTRACE("TMsgAuth1NewLoginRequest::Unpack");
	TMsgAuth1LoginBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	    (GetMessageType() != WONMsg::Auth1NewLoginRequest))
	{
		WDBG_AH("TMsgAuth1NewLoginRequest::Unpack Not a Auth1NewLoginRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1NewLoginRequest message.");
	}

	WDBG_LL("TMsgAuth1NewLoginRequest::Unpack Reading message data");
	ReadWString(mNickname);
	UnpackRawBuf();
}

#endif