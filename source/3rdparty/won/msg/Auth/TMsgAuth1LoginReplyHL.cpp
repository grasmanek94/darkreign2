// TMsgAuth1LoginReplyHL.h

// AuthServer login reply for Auth1 Half-Life login protocols.  This is the final
// message of the login exchange that delivers the Auth Certificate or failure
// information.  It may optionally deliver a new Auth Public Key Block, Client
// Private Key, secret confirmation, and/or extended error information

// This message contains an array of data.  The array may be composed as follows:
//	1) 0 or 1 Auth Certificate
//	2) 0 or 1 Client PrivateKey   (Encrypted with session key)
//	3) 0 or 1 Auth PublicKey Block
//	4) 0 or 1 Secret confirmation (Encrypted with session key)
//	5) 0 or more error strings.

// All items if present will be in the above order.  (1), (2), and (4) will only be
// present if status implies success.  (5) will only be present if status implies
// failure.  (3) may be present in all cases.

// The Secret confirmation contains:
//     2 byte random pad
//     Session key
// If the Session key in the message matches the
// session key sent in the Auth1Login message, the
// client knows that the data it received came from
// the auth server, and is not simply garbage.

// The Display Info contains:
//     1 byte flag
//     Nickname
// If the 1 byte flag is true, The NicknameKey in the
// Auth1LoginRequest was not found, and the Nickname
// is the default value.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1LoginReplyHL.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::TMsgAuth1LoginReplyHL;
};


// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginReplyHL::TMsgAuth1LoginReplyHL(WONMsg::ServiceType theServType) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mErrList(),
	mRawMap(),
	mBufMap()
{
	SetServiceType(theServType);
	SetMessageType(WONMsg::Auth1LoginReplyHL);
}


// TMessage ctor
TMsgAuth1LoginReplyHL::TMsgAuth1LoginReplyHL(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mErrList(),
	mRawMap(),
	mBufMap()
{
	Unpack();
}


// Copy ctor
TMsgAuth1LoginReplyHL::TMsgAuth1LoginReplyHL(const TMsgAuth1LoginReplyHL& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mErrList(theMsgR.mErrList),
	mRawMap(),
	mBufMap()
{
	// Copy buffers
	CopyBlocks(theMsgR);
}


// Destructor
TMsgAuth1LoginReplyHL::~TMsgAuth1LoginReplyHL()
{}


// ** Private Methods **

TMsgAuth1LoginReplyHL::RawBlock&
TMsgAuth1LoginReplyHL::GetBlockRef(EntryType theType)
{
	RawBlockMap::iterator aSrch(mRawMap.find(theType));
	if (aSrch == mRawMap.end())
	{
		mRawMap[theType] = RawBlock(NULL, 0);
		return mRawMap[theType];
	}
	else
		return aSrch->second;
}


void
TMsgAuth1LoginReplyHL::CopyBlocks(const TMsgAuth1LoginReplyHL& theMsgR)
{
	WTRACE("TMsgAuth1LoginReplyHL::CopyBlocks");
	RawBlockMap::const_iterator anItr(theMsgR.mRawMap.begin());
	for (; anItr != theMsgR.mRawMap.end(); anItr++)
		SetRawBlock(anItr->first, anItr->second.first, anItr->second.second, true);
}


void
TMsgAuth1LoginReplyHL::PackBlock(EntryType theType, RawBlock& theBlockR)
{
	WTRACE("TMsgAuth1LoginReplyHL::PackBlock");
	if (theBlockR.first)
	{
		WDBG_LL("TMsgAuth1LoginReplyHL::Pack Appending block, type=" << theType << "  Len=" << theBlockR.second);
		AppendByte(static_cast<unsigned char>(theType));
		AppendShort(theBlockR.second);
		AppendBytes(theBlockR.second, theBlockR.first);
	}
	else
		WDBG_LL("TMsgAuth1LoginReplyHL::Pack Ignore NULL block, type=" << theType);
}


void
TMsgAuth1LoginReplyHL::UnpackBlock(EntryType theType)
{
	WTRACE("TMsgAuth1LoginReplyHL::UnpackBlock");

	// Get block ref and read block length
	RawBlock&      aBlockR   = GetBlockRef(theType);
	unsigned short aBlockLen = ReadShort();

	// Error if block already set or data is 0
	if ((aBlockR.first) || (aBlockLen == 0))
	{
		WDBG_AH("TMsgAuth1LoginReplyHL::UnpackBlock Block already set or read length is 0.");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Auth1Login reply, block already set or buffer length of 0.");
	}

	WDBG_LL("TMsgAuth1LoginReplyHL::UnpackBlock Unpack raw block, len=" << aBlockLen);
	aBlockR.second = aBlockLen;
	aBlockR.first  = (aBlockLen == 0 ? NULL : static_cast<const unsigned char*>(ReadBytes(aBlockLen)));
}


// ** Public Methods **

// Assignment operator
TMsgAuth1LoginReplyHL&
TMsgAuth1LoginReplyHL::operator=(const TMsgAuth1LoginReplyHL& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mStatus  = theMsgR.mStatus;
		mErrList = theMsgR.mErrList;
		CopyBlocks(theMsgR);
	}
	return *this;
}


// TMsgAuth1LoginReplyHL::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginReplyHL::Pack(void)
{
	WTRACE("TMsgAuth1LoginReplyHL::Pack");
//	SetMessageType(WONMsg::Auth1LoginReplyHL);
	TMessage::Pack();

    MsgTypeAuth	aRtn = (MsgTypeAuth) GetMessageType();

	// Calc num entries
	unsigned short aNumEntries = mErrList.size();
	RawBlockMap::iterator anItr(mRawMap.begin());
	for (; anItr != mRawMap.end(); anItr++)
		if (anItr->second.first) aNumEntries++;

	WDBG_LL("TMsgAuth1LoginReplyHL::Pack Appending message data, NumEntries=" << aNumEntries);
	AppendShort(static_cast<unsigned short>(mStatus));
	AppendShort(aNumEntries);

	WDBG_LL("TMsgAuth1LoginReplyHL::Pack Appending blocks, ct=" << mRawMap.size());
	for (anItr=mRawMap.begin(); anItr != mRawMap.end(); anItr++)
		PackBlock(anItr->first, anItr->second);

	WDBG_LL("TMsgAuth1LoginReplyHL::Pack Appending Error info, ct=" << mErrList.size());
	ErrorList::iterator anErrItr(mErrList.begin());
	for (; anErrItr != mErrList.end(); anErrItr++)
	{
		AppendByte(static_cast<unsigned char>(LRErrorInfo));
		Append_PA_STRING(*anErrItr);
	}

	return GetDataPtr();
}


// TMsgAuth1LoginReplyHL::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1LoginReplyHL::Unpack(void)
{
	WTRACE("TMsgAuth1LoginReplyHL::Unpack");
	TMessage::Unpack();

	if (GetMessageType() != WONMsg::Auth1LoginReplyHL
	 && GetMessageType() != WONMsg::Auth1LoginReplyHL23)
	{
		WDBG_AH("TMsgAuth1LoginReplyHL::Unpack Not a Auth1LoginReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginReply message.");
	}

	// Clear buffers
	WDBG_LL("TMsgAuth1LoginReplyHL::Unpack Clearing buffers.");
	mErrList.clear();
	mRawMap.clear();

	WDBG_LL("TMsgAuth1LoginReplyHL::Unpack Reading status");
	mStatus = static_cast<ServerStatus>(static_cast<short>(ReadShort()));

	unsigned short aNumEntries = ReadShort();
	WDBG_LL("TMsgAuth1LoginReplyHL::Unpack Reading Entries, ct=" << aNumEntries);
	for (int i=0; i < aNumEntries; i++)
	{
		unsigned char aType = ReadByte();
		switch (aType)
		{
		case LRCertificate:
		case LRClientPrivateKey:
		case LRPublicKeyBlock:
		case LRSecretConfirm:
			WDBG_LL("TMsgAuth1LoginReplyHL::Unpack Raw Block.");
			UnpackBlock(static_cast<EntryType>(aType));
			break;
		case LRErrorInfo:
			WDBG_LL("TMsgAuth1LoginReplyHL::Unpack Unpack error info.");
			mErrList.push_back(Read_PA_STRING());
			break;
		default:
			WDBG_AH("TMsgAuth1LoginReplyHL::Unpack Unknown entry type!");
			throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
										  "Auth1LoginReply, unknown entry type!");
			break;
		}
	}
}


// Update a Block
void
TMsgAuth1LoginReplyHL::SetRawBlock(EntryType theType, const unsigned char* theBlockP,
                                   unsigned short theLen, bool copyBlock)
{
	WTRACE("TMsgAuth1LoginReplyHL::SetBuffer");

	// No setting error data this way
	if (theType == LRErrorInfo) 
	{
		WDBG_AH("TMsgAuth1LoginReplyHL::SetBuffer Set for ErrorInfo ignored!");
		return;
	}

	RawBlock& aBlockR = GetBlockRef(theType);
	if (! copyBlock)
	{
		WDBG_LL("TMsgAuth1LoginReplyHL::SetBuffer Assign buf (no copy!)");
		aBlockR.first  = theBlockP; 
		aBlockR.second = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuth1LoginReplyHL::SetBuffer Copy buf");
		aBlockR.second = theLen;
		if ((theLen > 0) && (theBlockP))
		{
			mBufMap[theType].assign(theBlockP, theLen);
			aBlockR.first = mBufMap[theType].data();
		}
		else
			aBlockR.first = NULL;
	}
}


void
TMsgAuth1LoginReplyHL::ForceOwn(EntryType theType)
{
	WTRACE("TMsgAuth1LoginReplyHL::ForceOwn");
	RawBlock& aBlockR = GetBlockRef(theType);

	if ((aBlockR.first) && (aBlockR.first != mBufMap[theType].data()))
	{ 
		WDBG_LL("TMsgAuth1LoginReplyHL::ForceOwn Force own buf for type=" << theType);
		mBufMap[theType].assign(aBlockR.first, aBlockR.second);
		aBlockR.first = mBufMap[theType].data();
	} 
}


void
TMsgAuth1LoginReplyHL::ForceOwnAll()
{
	WTRACE("TMsgAuth1LoginReplyHL::ForceAllOwn");
	RawBlockMap::iterator anItr(mRawMap.begin());
	for (; anItr != mRawMap.end(); anItr++)
		ForceOwn(anItr->first);
}