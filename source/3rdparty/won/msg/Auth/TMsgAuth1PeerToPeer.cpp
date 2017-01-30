// TMsgAuth1PeerToPeer.h

// AuthServer messages implementing the Auth1 Family peer-to-peer protocol.  These
// messages support both the persistent and lightweight Authentication protocols
// between clients/servers

// This header/source implements messages:
//	Auth1Request
//	Auth1Challenge1
//	Auth1Challenge2
//	Auth1Complete

// Auth1Request begins authentication bwteen two clients/servers and is sent from
// Client A to Client B.

// Auth1Challenge1 is sent in response to Auth1Request from Client B to Client A.

// Auth1Challenge2 is sent in response to Auth1Challenge1 from Client A to Client B.

// Auth1Complete is sent in response to Auth1Challenge2 from Client B to Client A and
// completes the authentication.



#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1PeerToPeer.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONMsg::TMessage;
	using WONMsg::TMsgAuthRawBufferBase;
	using WONMsg::ServerStatus;
	using WONMsg::TMsgAuth1Request;
	using WONMsg::TMsgAuth1Challenge1;
	using WONMsg::TMsgAuth1Challenge2;
	using WONMsg::TMsgAuth1Complete;
};


// ** TMsgAuth1Request **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1Request::TMsgAuth1Request() :
	TMsgAuthRawBufferBase(),
	mAuthMode(WONAuth::AUTH_PERSISTENT),
	mEncryptMode(WONAuth::ENCRYPT_NONE),
	mEncryptFlags(WONAuth::EFLAGS_NONE)
{
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Request);
}


// TMessage ctor
TMsgAuth1Request::TMsgAuth1Request(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mAuthMode(WONAuth::AUTH_PERSISTENT),
	mEncryptMode(WONAuth::ENCRYPT_NONE),
	mEncryptFlags(WONAuth::EFLAGS_NONE)
{
	Unpack();
}


// Copy ctor
TMsgAuth1Request::TMsgAuth1Request(const TMsgAuth1Request& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mAuthMode(theMsgR.mAuthMode),
	mEncryptMode(theMsgR.mEncryptMode),
	mEncryptFlags(theMsgR.mEncryptFlags)
{}


// Destructor
TMsgAuth1Request::~TMsgAuth1Request()
{}


// ** Public Methods **

// Assignment operator
TMsgAuth1Request&
TMsgAuth1Request::operator=(const TMsgAuth1Request& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		mAuthMode     = theMsgR.mAuthMode;
		mEncryptMode  = theMsgR.mEncryptMode;
		mEncryptFlags = theMsgR.mEncryptFlags;
	}
	return *this;
}


// TMsgAuth1Request::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1Request::Pack(void)
{
	WTRACE("TMsgAuth1Request::Pack");
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Request);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1Request::Pack Appending message data");
	AppendByte(static_cast<unsigned char>(mAuthMode));
	AppendByte(static_cast<unsigned char>(mEncryptMode));
	AppendShort(mEncryptFlags);
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1Request::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1Request::Unpack(void)
{
	WTRACE("TMsgAuth1Request::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1PeerToPeer) ||
	    (GetMessageType() != WONMsg::Auth1Request))
	{
		WDBG_AH("TMsgAuth1Request::Unpack Not a Auth1Request message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1Request message.");
	}

	WDBG_LL("TMsgAuth1Request::Unpack Reading message data");
	mAuthMode    = static_cast<AuthenticationMode>(ReadByte());
	mEncryptMode = static_cast<EncryptionMode>(ReadByte());
	mEncryptFlags = ReadShort();
	UnpackRawBuf();
}


// ** TMsgAuth1Challenge1 **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1Challenge1::TMsgAuth1Challenge1() :
	TMsgAuthRawBufferBase(),
	mSecretBP(NULL),
	mSecretBLen(0),
	mSecretBBuf()
{
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Challenge1);
}


// TMessage ctor
TMsgAuth1Challenge1::TMsgAuth1Challenge1(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mSecretBP(NULL),
	mSecretBLen(0),
	mSecretBBuf()
{
	Unpack();
}


// Copy ctor
TMsgAuth1Challenge1::TMsgAuth1Challenge1(const TMsgAuth1Challenge1& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mSecretBP(NULL),
	mSecretBLen(0),
	mSecretBBuf()
{
	SetSecretB(theMsgR.mSecretBP, theMsgR.mSecretBLen, true);
}


// Destructor
TMsgAuth1Challenge1::~TMsgAuth1Challenge1()
{}


// ** Public Methods **

// Assignment operator
TMsgAuth1Challenge1&
TMsgAuth1Challenge1::operator=(const TMsgAuth1Challenge1& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		SetSecretB(theMsgR.mSecretBP, theMsgR.mSecretBLen, true);
	}
	return *this;
}


void
TMsgAuth1Challenge1::SetSecretB(const unsigned char* theDataP, unsigned short theLen,
                                bool copyData)
{
	WTRACE("TMsgAuth1Challenge1::SetSecretB");
	if (! copyData)
	{
		WDBG_LL("TMsgAuth1Challenge1::SetSecretB Assign buf (no copy!)");
		mSecretBP   = theDataP; 
		mSecretBLen = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuth1Challenge1::SetSecretB Copy buf");
		mSecretBLen = theLen;
		if ((mSecretBLen > 0) && (theDataP))
		{
			mSecretBBuf.assign(theDataP, mSecretBLen);
			mSecretBP = mSecretBBuf.data();
		}
		else
			mSecretBP = NULL;
	}
}


// TMsgAuth1Challenge1::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1Challenge1::Pack(void)
{
	WTRACE("TMsgAuth1Challenge1::Pack");
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Challenge1);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1Challenge1::Pack Appending message data");
	AppendShort(mSecretBLen);
	if (mSecretBLen > 0) AppendBytes(mSecretBLen, mSecretBP);
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1Challenge1::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1Challenge1::Unpack(void)
{
	WTRACE("TMsgAuth1Request::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1PeerToPeer) ||
	    (GetMessageType() != WONMsg::Auth1Challenge1))
	{
		WDBG_AH("TMsgAuth1Challenge1::Unpack Not a Auth1Challenge1 message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1Challenge1 message.");
	}

	WDBG_LL("TMsgAuth1Challenge1::Unpack Reading message data");
	mSecretBLen = ReadShort();  // Get length
	mSecretBP   = (mSecretBLen == 0 ? NULL
	                              : static_cast<const unsigned char*>(ReadBytes(mSecretBLen)));

	UnpackRawBuf();
}


// ** TMsgAuth1Challenge2 **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1Challenge2::TMsgAuth1Challenge2(void) :
	TMsgAuthRawBufferBase()
{
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Challenge2);
}


// TMessage ctor
TMsgAuth1Challenge2::TMsgAuth1Challenge2(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1Challenge2::TMsgAuth1Challenge2(const TMsgAuth1Challenge2& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{}


// Destructor
TMsgAuth1Challenge2::~TMsgAuth1Challenge2(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1Challenge2&
TMsgAuth1Challenge2::operator=(const TMsgAuth1Challenge2& theMsgR)
{
	TMsgAuthRawBufferBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1Challenge2::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1Challenge2::Pack(void)
{
	WTRACE("TMsgAuth1Challenge2::Pack");
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Challenge2);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1Challenge2::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1Challenge2::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1Challenge2::Unpack(void)
{
	WTRACE("TMsgAuth1Challenge2::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1PeerToPeer) ||
	    (GetMessageType() != WONMsg::Auth1Challenge2))
	{
		WDBG_AH("TMsgAuth1Challenge2::Unpack Not a Auth1Challenge2 message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1Challenge2 message.");
	}

	WDBG_LL("TMsgAuth1Challenge2::Unpack Reading message data");
	UnpackRawBuf();
}


// ** TMsgAuth1Complete **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1Complete::TMsgAuth1Complete() :
	TMsgAuthRawBufferBase(),
	mStatus(WONMsg::StatusCommon_Success),
	mSessionId(0),
	mErrList()
{
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Complete);
}


// TMessage ctor
TMsgAuth1Complete::TMsgAuth1Complete(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mSessionId(0),
	mErrList()
{
	Unpack();
}


// Copy ctor
TMsgAuth1Complete::TMsgAuth1Complete(const TMsgAuth1Complete& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mStatus(theMsgR.mStatus),
	mSessionId(theMsgR.mSessionId),
	mErrList(theMsgR.mErrList)
{}


// Destructor
TMsgAuth1Complete::~TMsgAuth1Complete()
{}


// ** Public Methods **

// Assignment operator
TMsgAuth1Complete&
TMsgAuth1Complete::operator=(const TMsgAuth1Complete& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		mStatus    = theMsgR.mStatus;
		mSessionId = theMsgR.mSessionId;
		mErrList   = theMsgR.mErrList;
	}
	return *this;
}


// TMsgAuth1Complete::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1Complete::Pack(void)
{
	WTRACE("TMsgAuth1Complete::Pack");
	SetServiceType(WONMsg::Auth1PeerToPeer);
	SetMessageType(WONMsg::Auth1Complete);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1Complete::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	// Append error info if status implies failure
	if (mStatus < 0)
	{
		WDBG_LL("TMsgAuth1Complete::Pack Failure status, append error info");
		AppendShort(mErrList.size());
		ErrorList::iterator anItr(mErrList.begin());
		for (; anItr != mErrList.end(); anItr++)
			Append_PA_STRING(*anItr);
	}

	// Otherwise append success info
	else
	{
		WDBG_LL("TMsgAuth1Complete::Pack Success status, append secret and optional session");
		PackRawBuf();
		if (mSessionId != 0) AppendShort(mSessionId);
	}

	return GetDataPtr();
}


// TMsgAuth1Complete::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1Complete::Unpack(void)
{
	WTRACE("TMsgAuth1Complete::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1PeerToPeer) ||
	    (GetMessageType() != WONMsg::Auth1Complete))
	{
		WDBG_AH("TMsgAuth1Complete::Unpack Not a Auth1Complete message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1Complete message.");
	}

	WDBG_LL("TMsgAuth1Complete::Unpack Reading message data");
	mStatus = static_cast<ServerStatus>((short)ReadShort());

	// If status implies failure, read error info
	if (mStatus < 0)
	{
		WDBG_LL("TMsgAuth1Complete::Unpack Failure status, read error info");
		unsigned short aNumErrs = ReadShort();
		for (int i=0; i < aNumErrs; i++)
			mErrList.push_back(Read_PA_STRING());
		SetRawBuf(NULL, 0);
		mSessionId = 0;
	}

	// Otherwise, read secret and optional sessionId
	else
	{
		WDBG_LL("TMsgAuth1Complete::Unpack Success status, read secret and optional session");
		UnpackRawBuf();
		mSessionId = (BytesLeftToRead() > 0 ? ReadShort() : 0);
		mErrList.clear();
	}
}
