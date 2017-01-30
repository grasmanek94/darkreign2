// TMsgAuth1LoginHL.h

// AuthServer messages implementing the Auth1 Family login Half-Life protocol.
// This header/source implements messages:
//	Auth1LoginRequestHL
//	Auth1ChallengeHL
//	Auth1ConfirmHL
// It also included the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequestHL begins login for Half-Life and is sent from the Client to
// the AuthServer.

// Auth1ChallengeHL is sent in response to Auth1LoginRequest from the AuthServer
// to the client.

// Auth1RefreshHL is from the Client to the AuthServer to request extension of an
// Login session and AuthCertificate.

// Auth1LoginReply is sent in response to Auth1ConfirmHL from the AuthServer
// to the client and completes the login. It is also sent in response to
// Auth1RefreshHL, extending the login session.

#include "common/won.h"
#include "crypt/EGPrivateKey.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1LoginHL.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgAuth1LoginRequestHL;
	using WONMsg::TMsgAuth1ChallengeHL;
	using WONMsg::TMsgAuth1ConfirmHL;
	using WONMsg::TMsgAuth1RefreshHL;
	using WONMsg::TMsgAuth1CheckHLKey;
};


// ** TMsgAuth1LoginRequestHL **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginRequestHL::TMsgAuth1LoginRequestHL() :
	TMsgAuthRawBufferBase(),
	mKeyBlockId(0),
	mNeedKey(false)
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1LoginRequestHL);
}


// TMessage ctor
TMsgAuth1LoginRequestHL::TMsgAuth1LoginRequestHL(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mKeyBlockId(0),
	mNeedKey(false)
{
	Unpack();
}


// Copy ctor
TMsgAuth1LoginRequestHL::TMsgAuth1LoginRequestHL(const TMsgAuth1LoginRequestHL& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR),
	mKeyBlockId(theMsgR.mKeyBlockId),
	mNeedKey(theMsgR.mNeedKey)
{}


// Destructor
TMsgAuth1LoginRequestHL::~TMsgAuth1LoginRequestHL(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginRequestHL&
TMsgAuth1LoginRequestHL::operator=(const TMsgAuth1LoginRequestHL& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthRawBufferBase::operator=(theMsgR);
		mKeyBlockId = theMsgR.mKeyBlockId;
		mNeedKey    = theMsgR.mNeedKey;
	}
	return *this;
}


// TMsgAuth1LoginRequestHL::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginRequestHL::Pack(void)
{
	WTRACE("TMsgAuth1LoginRequestHL::Pack");
	SetServiceType(WONMsg::Auth1LoginHL);
//	SetMessageType(WONMsg::Auth1LoginRequestHL);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1LoginRequestHL::Pack Appending message data");
	AppendShort(mKeyBlockId);
	AppendByte(mNeedKey ? 1 : 0);
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1LoginRequestHL::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1LoginRequestHL::Unpack(void)
{
	WTRACE("TMsgAuth1LoginRequestHL::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1LoginHL)
	 || ( (GetMessageType() != WONMsg::Auth1LoginRequestHL)
	   && (GetMessageType() != WONMsg::Auth1LoginRequestHL23)) )
	{
		WDBG_AH("TMsgAuth1LoginRequestHL::Unpack Not a Auth1LoginRequestHL message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginRequestHL message.");
	}

	WDBG_LL("TMsgAuth1LoginRequestHL::Unpack Reading message data");
	mKeyBlockId = ReadShort();
	mNeedKey    = (ReadByte() ? true : false);
	UnpackRawBuf();
}


// ** TMsgAuth1ChallengeHL **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1ChallengeHL::TMsgAuth1ChallengeHL(void) :
	TMsgAuthRawBufferBase()
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1LoginChallengeHL);
}


// TMessage ctor
TMsgAuth1ChallengeHL::TMsgAuth1ChallengeHL(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1ChallengeHL::TMsgAuth1ChallengeHL(const TMsgAuth1ChallengeHL& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{}


// Destructor
TMsgAuth1ChallengeHL::~TMsgAuth1ChallengeHL(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1ChallengeHL&
TMsgAuth1ChallengeHL::operator=(const TMsgAuth1ChallengeHL& theMsgR)
{
	TMsgAuthRawBufferBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1ChallengeHL::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1ChallengeHL::Pack(void)
{
	WTRACE("TMsgAuth1ChallengeHL::Pack");
	SetServiceType(WONMsg::Auth1LoginHL);
//	SetMessageType(WONMsg::Auth1LoginChallengeHL);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1ChallengeHL::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1ChallengeHL::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1ChallengeHL::Unpack(void)
{
	WTRACE("TMsgAuth1ChallengeHL::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1LoginHL)
	 || ( (GetMessageType() != WONMsg::Auth1LoginChallengeHL)
	   && (GetMessageType() != WONMsg::Auth1LoginChallengeHL23)) )
	{
		WDBG_AH("TMsgAuth1ChallengeHL::Unpack Not a Auth1LoginChallengeHL message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginChallengeHL message.");
	}

	WDBG_LL("TMsgAuth1ChallengeHL::Unpack Reading message data");
	UnpackRawBuf();
}


// ** TMsgAuth1ConfirmHL **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1ConfirmHL::TMsgAuth1ConfirmHL(void) :
	TMsgAuthRawBufferBase()
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1LoginConfirmHL);
}


// TMessage ctor
TMsgAuth1ConfirmHL::TMsgAuth1ConfirmHL(const TMessage& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgAuth1ConfirmHL::TMsgAuth1ConfirmHL(const TMsgAuth1ConfirmHL& theMsgR) :
	TMsgAuthRawBufferBase(theMsgR)
{}


// Destructor
TMsgAuth1ConfirmHL::~TMsgAuth1ConfirmHL(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1ConfirmHL&
TMsgAuth1ConfirmHL::operator=(const TMsgAuth1ConfirmHL& theMsgR)
{
	TMsgAuthRawBufferBase::operator=(theMsgR);
	return *this;
}


// TMsgAuth1ConfirmHL::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1ConfirmHL::Pack(void)
{
	WTRACE("TMsgAuth1ConfirmHL::Pack");
	SetServiceType(WONMsg::Auth1LoginHL);
//	SetMessageType(WONMsg::Auth1LoginConfirmHL);
	TMsgAuthRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1ConfirmHL::Pack Appending message data");
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1ConfirmHL::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1ConfirmHL::Unpack(void)
{
	WTRACE("TMsgAuth1ConfirmHL::Unpack");
	TMsgAuthRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1LoginHL)
	 || ( (GetMessageType() != WONMsg::Auth1LoginConfirmHL)
	   && (GetMessageType() != WONMsg::Auth1LoginConfirmHL23)) )
	{
		WDBG_AH("TMsgAuth1ConfirmHL::Unpack Not a Auth1LoginConfirmHL message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginConfirmHL message.");
	}

	WDBG_LL("TMsgAuth1ConfirmHL::Unpack Reading message data");
	UnpackRawBuf();
}

// ** TMsgAuth1RefreshHL **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1RefreshHL::TMsgAuth1RefreshHL() :
	TMsgAuthLastRawBufferBase(),
	mUserId(0)
{
	SetServiceType(WONMsg::Auth1LoginHL);
	SetMessageType(WONMsg::Auth1RefreshHL);
}


// TMessage ctor
TMsgAuth1RefreshHL::TMsgAuth1RefreshHL(const TMessage& theMsgR) :
	TMsgAuthLastRawBufferBase(theMsgR),
	mUserId(0)
{
	Unpack();
}


// Copy ctor
TMsgAuth1RefreshHL::TMsgAuth1RefreshHL(const TMsgAuth1RefreshHL& theMsgR) :
	TMsgAuthLastRawBufferBase(theMsgR),
	mUserId(theMsgR.mUserId)
{}


// Destructor
TMsgAuth1RefreshHL::~TMsgAuth1RefreshHL(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1RefreshHL&
TMsgAuth1RefreshHL::operator=(const TMsgAuth1RefreshHL& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMsgAuthLastRawBufferBase::operator=(theMsgR);
		mUserId = theMsgR.mUserId;
	}
	return *this;
}


// TMsgAuth1RefreshHL::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1RefreshHL::Pack(void)
{
	WTRACE("TMsgAuth1RefreshHL::Pack");
	SetServiceType(WONMsg::Auth1LoginHL);
//	SetMessageType(WONMsg::Auth1RefreshHL);
	TMsgAuthLastRawBufferBase::Pack();

	WDBG_LL("TMsgAuth1RefreshHL::Pack Appending message data");
	AppendLong(mUserId);
	PackRawBuf();

	return GetDataPtr();
}


// TMsgAuth1RefreshHL::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1RefreshHL::Unpack(void)
{
	WTRACE("TMsgAuth1RefreshHL::Unpack");
	TMsgAuthLastRawBufferBase::Unpack();

	if ((GetServiceType() != WONMsg::Auth1LoginHL)
	 || ( (GetMessageType() != WONMsg::Auth1RefreshHL)
	   && (GetMessageType() != WONMsg::Auth1RefreshHL23)) )
	{
		WDBG_AH("TMsgAuth1RefreshHL::Unpack Not a TMsgAuth1RefreshHL message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a TMsgAuth1RefreshHL message.");
	}

	WDBG_LL("TMsgAuth1RefreshHL::Unpack Reading message data");
	mUserId = ReadLong();
	UnpackRawBuf();
}


// ** TMsgAuth1CheckHLKey **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1CheckHLKey::TMsgAuth1CheckHLKey(void) :
	TMsgAuth1LoginBase2(), mNeedKeyFlg(false), mCreateAcctFlg(false), mKeyBuf(NULL), mDataBuf(NULL)
{
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1CheckHLKey);
}


// TMessage ctor
TMsgAuth1CheckHLKey::TMsgAuth1CheckHLKey(const TMessage& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR), mKeyBuf(NULL), mDataBuf(NULL)
{
	Unpack();
}


// Copy ctor
TMsgAuth1CheckHLKey::TMsgAuth1CheckHLKey(const TMsgAuth1CheckHLKey& theMsgR) :
	TMsgAuth1LoginBase2(theMsgR), mNeedKeyFlg(theMsgR.mNeedKeyFlg), mCreateAcctFlg(theMsgR.mCreateAcctFlg),
		mUserName(theMsgR.mUserName), mCommunityName(theMsgR.mCommunityName), mNicknameKey(theMsgR.mNicknameKey),
		mPassword(theMsgR.mPassword), mNewPassword(theMsgR.mNewPassword), mCDKey(theMsgR.mCDKey),
		mKeyBuf(NULL), mDataBuf(NULL)
{
}


// Destructor
TMsgAuth1CheckHLKey::~TMsgAuth1CheckHLKey(void)
{
	delete [] mKeyBuf;
	delete [] mDataBuf;
}


// ** Public Methods

// Assignment operator
TMsgAuth1CheckHLKey&
TMsgAuth1CheckHLKey::operator=(const TMsgAuth1CheckHLKey& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
	    TMsgAuth1LoginBase2::operator=(theMsgR);

			delete [] mKeyBuf; mKeyBuf = NULL;
			delete [] mDataBuf; mDataBuf = NULL;

			mNeedKeyFlg = theMsgR.mNeedKeyFlg;
			mCreateAcctFlg = theMsgR.mCreateAcctFlg;
			mUserName = theMsgR.mUserName;
			mCommunityName = theMsgR.mCommunityName;
			mNicknameKey = theMsgR.mNicknameKey;
			mPassword = theMsgR.mPassword;
			mNewPassword = theMsgR.mNewPassword;
			mCDKey = theMsgR.mCDKey;
	}
	return *this;
}

// Build the raw encrypted buffers from class data members
bool TMsgAuth1CheckHLKey::BuildBuffer(const WONAuth::Auth1PublicKeyBlock &thePubKeyBlock, 
																					const WONCrypt::BFSymmetricKey &theSessionKey)
{
	SetRawKeyBuf(NULL,0,false);
	SetRawDataBuf(NULL,0,false);
	delete [] mKeyBuf; mKeyBuf = NULL;
	delete [] mDataBuf; mDataBuf = NULL;

	try {
		if(!thePubKeyBlock.IsValid()) 
			return false;

		SetKeyBlockId(thePubKeyBlock.GetBlockId());
		WONCrypt::CryptKeyBase::CryptReturn aCryptRet(NULL,0);
		
		aCryptRet = thePubKeyBlock.EncryptRawBuffer(theSessionKey.GetKey(),theSessionKey.GetKeyLen());
		if(aCryptRet.first==NULL) 
			return false;

		mKeyBuf = aCryptRet.first;
		SetRawKeyBuf(aCryptRet.first,aCryptRet.second,false);
		aCryptRet.first = NULL;

		TRawMsg aBuf;
		aBuf.AppendShort(thePubKeyBlock.GetBlockId());
		aBuf.AppendByte(mNeedKeyFlg);
		aBuf.AppendByte(mCreateAcctFlg);
		aBuf.Append_PW_STRING(mUserName);
		aBuf.Append_PW_STRING(mCommunityName);
		aBuf.Append_PW_STRING(mNicknameKey);
		aBuf.Append_PW_STRING(mPassword);
		aBuf.Append_PW_STRING(mNewPassword);
		aBuf.AppendShort(mCDKey.size());
		aBuf.AppendBytes(mCDKey.size(),mCDKey.data());

		aCryptRet = theSessionKey.Encrypt(aBuf.GetDataPtr(),aBuf.GetDataLen());
		if(aCryptRet.first==NULL)
			return false;

		mDataBuf = aCryptRet.first;
		SetRawDataBuf(aCryptRet.first,aCryptRet.second,false);
		aCryptRet.first = NULL;
	}
	catch(WONCommon::WONException&) {
		return false;
	}

	return true;
}


bool TMsgAuth1CheckHLKey::ExtractBuffer(const WONCrypt::EGPrivateKey &thePrivateKey, 
																		 WONCrypt::BFSymmetricKey *theSessionKey)
{
	try {
		WONCrypt::CryptKeyBase::CryptReturn aCryptRet(NULL,0);

		aCryptRet = thePrivateKey.Decrypt(GetRawKeyBuf(),GetRawKeyBufLen());
		auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

		if(aCryptRet.first==NULL) 
			return false;

		theSessionKey->Create(aCryptRet.second,aCryptRet.first);
		
		aCryptRet.first=NULL;
		aCryptRet = theSessionKey->Decrypt(GetRawDataBuf(),GetRawDataBufLen());
		if(aCryptRet.first==NULL)
			return false;

		TRawMsg aBuf(aCryptRet.second, aCryptRet.first);
		int aBlockId = aBuf.ReadShort();
		if(aBlockId!=GetKeyBlockId())
			return false;

		mNeedKeyFlg = aBuf.ReadByte()!=0;
		mCreateAcctFlg = aBuf.ReadByte()!=0;
		aBuf.ReadWString(mUserName);
		aBuf.ReadWString(mCommunityName);
		aBuf.ReadWString(mNicknameKey);
		aBuf.ReadWString(mPassword);
		aBuf.ReadWString(mNewPassword);
	
		unsigned short aLen;
		aLen = aBuf.ReadShort();
		mCDKey.assign((const unsigned char*)aBuf.ReadBytes(aLen),aLen);
	}
	catch(WONCommon::WONException&) {
		return false;
	}

	return true;
}


// TMsgAuth1CheckHLKey::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1CheckHLKey::Pack(void)
{
	WTRACE("TMsgAuth1CheckHLKey::Pack");
	SetServiceType(WONMsg::Auth1Login);
	SetMessageType(WONMsg::Auth1CheckHLKey);
	TMsgAuth1LoginBase2::Pack();

	return GetDataPtr();
}


// TMsgAuth1CheckHLKey::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
// Note: call ForceRawBufOwn() to force ownership of the data buffers.
void
TMsgAuth1CheckHLKey::Unpack(void)
{
	WTRACE("TMsgAuth1CheckHLKey::Unpack");
	TMsgAuth1LoginBase2::Unpack();

	if ((GetServiceType() != WONMsg::Auth1Login) ||
	    (GetMessageType() != WONMsg::Auth1CheckHLKey))
	{
		WDBG_AH("TMsgAuth1CheckHLKey::Unpack Not a Auth1CheckHLKey message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1CheckHLKey message.");
	}

}