// TMsgAuth1Login.h

// AuthServer messages implementing the Auth1 Family login protocol.
// This header/source implements messages:
//	Auth1LoginRequest
//	Auth1NewLoginRequest
// It also includes the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequest begins login for an existing user and is sent from the Client to
// the AuthServer.

// Auth1LoginReply is sent in response to both Auth1LoginRequest and
// Auth1NewLoginRequest from the AuthServer to the client and completes the login.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1Login2.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgAuth1LoginRequest2;
};


// ** TMsgAuth1LoginRequest2 **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginRequest2::TMsgAuth1LoginRequest2(void) :
	TMsgAuth1LoginBase2()
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1LoginRequest2);
}


// TMessage ctor
TMsgAuth1LoginRequest2::TMsgAuth1LoginRequest2(const TMessage& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1LoginRequest2::TMsgAuth1LoginRequest2(const TMsgAuth1LoginRequest2& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR)
{
}


// Destructor
TMsgAuth1LoginRequest2::~TMsgAuth1LoginRequest2(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginRequest2&
TMsgAuth1LoginRequest2::operator=(const TMsgAuth1LoginRequest2& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
	    TMsgAuth1LoginBase2::operator=(theMsgR);
	}
	return *this;
}


// TMsgAuth1LoginRequest2::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginRequest2::Pack(void)
{
	WTRACE("TMsgAuth1LoginRequest2::Pack");
//	SetServiceType(WONMsg::Auth1Login);
//	SetMessageType(WONMsg::Auth1LoginRequest2);
	TMsgAuth1LoginBase2::Pack();

	return GetDataPtr();
}


// TMsgAuth1LoginRequest2::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
// Note: call ForceRawBufOwn() to force ownership of the data buffers.
void
TMsgAuth1LoginRequest2::Unpack(void)
{
	WTRACE("TMsgAuth1LoginRequest2::Unpack");
	TMsgAuth1LoginBase2::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	    (GetMessageType() != WONMsg::Auth1LoginRequest2
      && GetMessageType() != WONMsg::Auth1LoginRequest223 ))
	{
		WDBG_AH("TMsgAuth1LoginRequest2::Unpack Not a Auth1LoginRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginRequest2 message.");
	}

}
