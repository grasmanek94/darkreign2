// TMsgAuth1LoginReply.cpp

// AuthServer login reply for many of the Auth1 login protocol. This is the final
// message is the login exchange that delivers the Auth Certificate or failure
// information.  It may optionally deliver other pieces of information as well which
// are described below.

// This message contains a status, followed by an option array of error strings,
// followed by an optional array of data encrypted with the session key.  Once
// decrypted, the array will be composed as follows:
//	1) 0 or 1 Auth Certificate
//	2) 0 or 1 Client PrivateKey
//	3) 0 or 1 Auth PublicKey Block
//	4) 0 or 1 Secret confirmation
//	5) 0 or 1 Nickname info

// If present, the secret confirmation contains:
//     2 byte random pad
//     Session key
// If the Session key in the message matches the session key sent in the Auth1Login
// message, the client knows that the data it received came from the auth server, and
// is not simply garbage.

// The Nickname Info contains:
//     1 byte flag
//     Nickname
// If the 1 byte flag is true, The NicknameKey in the Auth1LoginRequest was not found,
// and the Nickname is the default value.


#include "common/won.h"
#include "crypt/SymmetricKey.h"
#include "crypt/BFSymmetricKey.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1LoginReply.h"
#include "common/WONEndian.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONCrypt::SymmetricKey;
	using WONCrypt::BFSymmetricKey;
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::TMsgAuth1LoginReply;
};


// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginReply::TMsgAuth1LoginReply(WONMsg::ServiceType theServType) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mErrList(),
	mRawMap(),
	mBufMap(),
	mSessKeyP(NULL),
	mOwnKey(false),
	mDecrypt(NULL,0)
{
	SetServiceType(theServType);
	SetMessageType(WONMsg::Auth1LoginReply);
}


// TMessage ctor
TMsgAuth1LoginReply::TMsgAuth1LoginReply(const TMessage& theMsgR, SymmetricKey* theKeyP,
                                         bool copyKey) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mErrList(),
	mRawMap(),
	mBufMap(),
	mSessKeyP(NULL),
	mOwnKey(false),
	mDecrypt(NULL,0)
{
	SetSessKey(theKeyP, copyKey);
	Unpack();
}


// Copy ctor
TMsgAuth1LoginReply::TMsgAuth1LoginReply(const TMsgAuth1LoginReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mErrList(theMsgR.mErrList),
	mRawMap(),
	mBufMap(),
	mSessKeyP(NULL),
	mOwnKey(false),
	mDecrypt(NULL,0)
{
	// Copy key and buffers
	SetSessKey(theMsgR.mSessKeyP, true);
	CopyBlocks(theMsgR);
}


// Destructor
TMsgAuth1LoginReply::~TMsgAuth1LoginReply()
{
	delete mDecrypt.first;

	if (mOwnKey)
		delete mSessKeyP;
}


// ** Private Methods **

TMsgAuth1LoginReply::RawBlock&
TMsgAuth1LoginReply::GetBlockRef(EntryType theType)
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
TMsgAuth1LoginReply::CopyBlocks(const TMsgAuth1LoginReply& theMsgR)
{
	WTRACE("TMsgAuth1LoginReply::CopyBlocks");
	RawBlockMap::const_iterator anItr(theMsgR.mRawMap.begin());
	for (; anItr != theMsgR.mRawMap.end(); anItr++)
		SetRawBlock(anItr->first, anItr->second.first, anItr->second.second, true);
}

void
TMsgAuth1LoginReply::PackClearBlock(EntryType theType, RawBlock& theBlockR)
{
	WTRACE("TMsgAuth1LoginReply::PackClearBlock");
	if (theBlockR.first)
	{
		WDBG_LL("TMsgAuth1LoginReply::PackClearBlock Appending block, type=" << theType << "  Len=" << theBlockR.second);
        AppendByte( static_cast<unsigned char>(theType) );
		AppendShort(theBlockR.second);
		AppendBytes(theBlockR.second, theBlockR.first);
	}
	else
		WDBG_LL("TMsgAuth1LoginReply::PackClearBlock Ignore NULL block, type=" << theType);
}
void
TMsgAuth1LoginReply::PackBlock(RawBuffer& theBufR, EntryType theType, RawBlock& theBlockR)
{
	WTRACE("TMsgAuth1LoginReply::PackBlock");
	if (theBlockR.first)
	{
		WDBG_LL("TMsgAuth1LoginReply::PackBlock Appending block, type=" << theType << "  Len=" << theBlockR.second);
		theBufR += static_cast<unsigned char>(theType);
		unsigned short tmpBlockSize = getLittleEndian(theBlockR.second);
		theBufR.append(reinterpret_cast<unsigned char*>(&tmpBlockSize), sizeof(tmpBlockSize));
		theBufR.append(theBlockR.first, theBlockR.second);
	}
	else
		WDBG_LL("TMsgAuth1LoginReply::PackBlock Ignore NULL block, type=" << theType);
}


void
TMsgAuth1LoginReply::EncryptAndPack()
{
	WTRACE("TMsgAuth1LoginReply::EncryptAndPack");
	RawBuffer aPackBuf;

	// Calc num entries
	unsigned char aNumCryptEntries = 0;
	unsigned char aNumClearEntries = 0;

	RawBlockMap::iterator anItr(mRawMap.begin());
	for (; anItr != mRawMap.end(); anItr++ )
    {
		if (anItr->second.first)
        {
            switch( anItr->first )
            {
            case ALCertificate:
            case AL2Certificate:
            case ALPublicKeyBlock:
                            aNumClearEntries++;
                            break;

            case ALClientPrivateKey:
            case ALSecretConfirm:
            case ALNicknameInfo:
                            aNumCryptEntries++;
                            break;

            default:
		        WDBG_AH("TMsgAuth1LoginReply::EncryptAndPack bad block type.");
		        throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                                      "Auth1Login reply, bad block type.");
            }
        }
    }

	WDBG_LL("TMsgAuth1LoginReply::EncryptAndPack Adding NumClearEntries=" << aNumClearEntries);
	WDBG_LL("TMsgAuth1LoginReply::EncryptAndPack Adding NumCryptEntries=" << aNumCryptEntries);
	aPackBuf += aNumCryptEntries;
    AppendByte( aNumClearEntries );

	WDBG_LL("TMsgAuth1LoginReply::EncryptAndPack Appending blocks, ct=" << mRawMap.size());
	for (anItr=mRawMap.begin(); anItr != mRawMap.end(); anItr++)
    {
        switch( anItr->first )
        {
        case ALCertificate:
        case AL2Certificate:
        case ALPublicKeyBlock:
            PackClearBlock( anItr->first, anItr->second );
            break;
        case ALClientPrivateKey:
        case ALSecretConfirm:
        case ALNicknameInfo:
	    	PackBlock(aPackBuf, anItr->first, anItr->second);
            break;
        }
    }

    if( aNumCryptEntries && mSessKeyP )
    {
        WDBG_LL("TMsgAuth1LoginReply::EncryptAndPack Encrypting buf, size=" << aPackBuf.size());
        BFSymmetricKey::CryptReturn aBlock(mSessKeyP->Encrypt(aPackBuf.data(), aPackBuf.size()));
        auto_ptr< unsigned char > anAP( aBlock.first );
        if (! aBlock.first)
        {
	        WDBG_AH("TMsgAuth1LoginReply::EncryptAndPack Encryption failed.");
	        throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                                  "Auth1Login reply, encryption failed.");
        }

        WDBG_LL("TMsgAuth1LoginReply::EncryptAndPack Appending encrypted block, size=" << aBlock.second);
        AppendBytes(aBlock.second, aBlock.first);
    }
    else
    {
        if( aNumCryptEntries )
        {
	        WDBG_AH("TMsgAuth1LoginReply::EncryptAndPack need session key for this data.");
	        throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                                  "Auth1Login reply, need session key for this data.");
        }
    }
}


void
TMsgAuth1LoginReply::UnpackBlock(EntryType theType, unsigned char*& theBufP,
                                 unsigned long& theLen)
{
	WTRACE("TMsgAuth1LoginReply::UnpackBlock");
	if (theLen < sizeof(unsigned short))
	{
		WDBG_AH("TMsgAuth1LoginReply::UnpackBlock Read exceeds buffer for len.");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Auth1Login reply, Read exceeds buffer for len.");
	}

	// Read block length
	unsigned short aBlockLen = *(reinterpret_cast<unsigned short*>(theBufP));
	makeLittleEndian(aBlockLen);
	theBufP += sizeof(unsigned short);  theLen -= sizeof(unsigned short);

	// Check block ref and block length
	RawBlock& aBlockR = GetBlockRef(theType);
	if ((aBlockR.first) || (aBlockLen == 0) || (theLen < aBlockLen))
	{
		WDBG_AH("TMsgAuth1LoginReply::UnpackBlock Block already set or read length is 0 or exceeds buffer.");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Auth1Login reply, block already set or length of 0 or exceeds buffer.");
	}

	WDBG_LL("TMsgAuth1LoginReply::UnpackBlock Unpack raw block, len=" << aBlockLen);
	aBlockR.second = aBlockLen;
	aBlockR.first  = theBufP;
	theBufP += aBlockLen;  theLen -= aBlockLen;
}

void
TMsgAuth1LoginReply::UnpackClearBlock(EntryType theType)
{
	WTRACE("TMsgAuth1LoginReply::UnpackClearBlock");

	// Read block length
	unsigned short aBlockLen = ReadShort();

	// Check block ref and block length
	RawBlock& aBlockR = GetBlockRef(theType);
	if ((aBlockR.first) || (aBlockLen == 0))
	{
		WDBG_AH("TMsgAuth1LoginReply::UnpackClearBlock Block already set or length of 0.");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Auth1Login reply, block already set or length of 0.");
	}

	WDBG_LL("TMsgAuth1LoginReply::UnpackClearBlock Unpack raw block, len=" << aBlockLen);
	aBlockR.second = aBlockLen;
	aBlockR.first  = static_cast<const unsigned char *>(ReadBytes(aBlockLen));
}

void
TMsgAuth1LoginReply::DecryptAndUnpack()
{
	WTRACE("TMsgAuth1LoginReply::DecryptAndUnpack");
    unsigned char aNumClearEntries = ReadByte();
	for (int i=0; i < aNumClearEntries; i++)
	{
        unsigned char aType = ReadByte();
		switch (aType)
		{
		case ALCertificate:
		case AL2Certificate:
		case ALPublicKeyBlock:
			WDBG_LL("TMsgAuthRawBufferBase::DecryptAndUnpack Clear Block.");
			UnpackClearBlock(static_cast<EntryType>(aType));
			break;
		case ALClientPrivateKey:
		case ALSecretConfirm:
		case ALNicknameInfo:
			WDBG_AH("TMsgAuth1LoginReply::DecryptAndUnpack bad entry type!");
			throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
										  "Auth1LoginReply, bad entry type!");
            break;
		default:
			WDBG_AH("TMsgAuth1LoginReply::DecryptAndUnpack Unknown entry type!");
			throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
										  "Auth1LoginReply, unknown entry type!");
			break;
		}
    }

	unsigned long aByteCt = BytesLeftToRead();
    if( aByteCt )
    {
    	// PUNT if no key
	    if (! mSessKeyP)
	    {
		    WDBG_AH("TMsgAuth1LoginReply::Unpack No Session Key!");
		    throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                                  "No Session to decrypt Auth1LoginReply message.");
	    }
	    WDBG_LL("TMsgAuth1LoginReply::DecryptAndUnpack Decrypting buf, size=" << aByteCt);

	    mDecrypt = mSessKeyP->Decrypt(static_cast<const unsigned char*>(ReadBytes(aByteCt)), aByteCt);
        if (mDecrypt.second < sizeof(char))
	    {
		    WDBG_AH("TMsgAuth1LoginReply::DecryptAndUnpack Decryption failed.");
		    throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                                  "Auth1Login reply, decryption failed.");
	    }

	    unsigned char* aBufP = mDecrypt.first;
	    unsigned long  aLen  = mDecrypt.second;

	    unsigned char aNumCryptEntries = *aBufP;
	    aBufP += sizeof(aNumCryptEntries);  aLen -= sizeof(aNumCryptEntries);
	    WDBG_LL("TMsgAuth1LoginReply::Unpack Reading Entries, ct=" << aNumCryptEntries);

	    for (int i=0; i < aNumCryptEntries; i++)
	    {
		    if (aLen < sizeof(unsigned char))
		    {
			    WDBG_AH("TMsgAuth1LoginReply::DecryptAndUnpack Read exceeds buffer.");
			    throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
										      "Auth1Login reply, read exceeds buffer.");
		    }
		    unsigned char aType = *aBufP;
		    aBufP += sizeof(unsigned char);  aLen -= sizeof(unsigned char);

		    switch (aType)
		    {
		    case ALClientPrivateKey:
		    case ALSecretConfirm:
		    case ALNicknameInfo:
			    WDBG_LL("TMsgAuthRawBufferBase::DecryptAndUnpack Raw Block.");
			    UnpackBlock(static_cast<EntryType>(aType), aBufP, aLen);
			    break;
		    case ALCertificate:
		    case AL2Certificate:
		    case ALPublicKeyBlock:
		    default:
			    WDBG_AH("TMsgAuth1LoginReply::DecryptAndUnpack Unknown entry type!");
			    throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
										      "Auth1LoginReply, unknown entry type!");
			    break;
		    }
	    }
    }
}


// ** Public Methods **

// Assignment operator
TMsgAuth1LoginReply&
TMsgAuth1LoginReply::operator=(const TMsgAuth1LoginReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mStatus  = theMsgR.mStatus;
		mErrList = theMsgR.mErrList;
		SetSessKey(theMsgR.mSessKeyP, true);
		CopyBlocks(theMsgR);
	}
	return *this;
}


// Assignment operator
void
TMsgAuth1LoginReply::SetSessKey(SymmetricKey* theKeyP, bool copyKey)
{
	WTRACE("TMsgAuth1LoginReply::SetSessKey");
	if (theKeyP)
	{
		if (mOwnKey)
			delete mSessKeyP;

		mOwnKey   = copyKey;
		if( copyKey )
		{
		    mSessKeyP = new BFSymmetricKey();
			mSessKeyP->Create( theKeyP->GetKeyLen(), theKeyP->GetKey() );
		}
		else
			mSessKeyP = theKeyP;
	}
}


// TMsgAuth1LoginReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginReply::Pack(void)
{
	WTRACE("TMsgAuth1LoginReply::Pack");
//	SetMessageType(WONMsg::Auth1LoginReply);
	TMessage::Pack();

	// Add status to reply
	WDBG_LL("TMsgAuth1LoginReply::Pack Appending status=" << mStatus);
	AppendShort(static_cast<unsigned short>(mStatus));

    AppendByte(static_cast<unsigned char>(mErrList.size()));
    if( mErrList.size() )
    {
	    WDBG_LL("TMsgAuth1LoginReplyHL::Pack Appending Error info, ct=" << mErrList.size());
	    ErrorList::iterator anErrItr(mErrList.begin());
	    for (; anErrItr != mErrList.end(); anErrItr++)
	    {
		    Append_PA_STRING(*anErrItr);
	    }
    }

	// Build and pack raw data and encrypted portions
    EncryptAndPack();

	return GetDataPtr();
}


// TMsgAuth1LoginReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgAuth1LoginReply::Unpack(void)
{
	WTRACE("TMsgAuth1LoginReply::Unpack");
	TMessage::Unpack();

	if (GetMessageType() != WONMsg::Auth1LoginReply)
	{
		WDBG_AH("TMsgAuth1LoginReply::Unpack Not a Auth1LoginReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a Auth1LoginReply message.");
	}

	// Clear buffers
	WDBG_LL("TMsgAuth1LoginReply::Unpack Clearing buffers.");
	mErrList.clear();
	mRawMap.clear();

	WDBG_LL("TMsgAuth1LoginReply::Unpack Reading status");
	mStatus = static_cast<ServerStatus>(static_cast<short>(ReadShort()));

    unsigned char anErrCt = ReadByte();
    if( anErrCt )
    {
		WDBG_LL("TMsgAuth1LoginReplyHL::Unpack reading error info, ct=" << anErrCt);
		for (int i=0; i < anErrCt; i++)
		{
			string anErrStr;
			ReadString(anErrStr);
			mErrList.push_back(anErrStr);
		}
	}

	// Decrypt and unpack
	DecryptAndUnpack();
}


// Update a Block
void
TMsgAuth1LoginReply::SetRawBlock(EntryType theType, const unsigned char* theBlockP,
                                 unsigned short theLen, bool copyBlock)
{
	WTRACE("TMsgAuth1LoginReply::SetBuffer");
	RawBlock& aBlockR = GetBlockRef(theType);
	if (! copyBlock)
	{
		WDBG_LL("TMsgAuth1LoginReply::SetBuffer Assign buf (no copy!)");
		aBlockR.first  = theBlockP; 
		aBlockR.second = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuth1LoginReply::SetBuffer Copy buf");
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
TMsgAuth1LoginReply::ForceOwn(EntryType theType)
{
	WTRACE("TMsgAuth1LoginReply::ForceOwn");
	RawBlock& aBlockR = GetBlockRef(theType);

	if ((aBlockR.first) && (aBlockR.first != mBufMap[theType].data()))
	{ 
		WDBG_LL("TMsgAuth1LoginReply::ForceOwn Force own buf for type=" << theType);
		mBufMap[theType].assign(aBlockR.first, aBlockR.second);
		aBlockR.first = mBufMap[theType].data();
	} 
}


void
TMsgAuth1LoginReply::ForceOwnAll()
{
	WTRACE("TMsgAuth1LoginReply::ForceAllOwn");
	RawBlockMap::iterator anItr(mRawMap.begin());
	for (; anItr != mRawMap.end(); anItr++)
		ForceOwn(anItr->first);
}