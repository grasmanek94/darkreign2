// TMsgAuth1LoginBase2.cpp


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesAuth.h"
#include "TMsgAuth1LoginBase2.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgAuth1LoginBase2;
};


// ** TMsgAuth1LoginBase2 **

// ** Constructors / Destructor **

// Default ctor
TMsgAuth1LoginBase2::TMsgAuth1LoginBase2(void) :
	TMessage(),
    mKeyBlockId( 0 ),
    mRawKeyP   ( NULL ),
    mRawKeyLen ( 0 ),
    mRawP      ( NULL ),
    mRawLen    ( 0 )
{
}


// TMessage ctor
TMsgAuth1LoginBase2::TMsgAuth1LoginBase2(const TMessage& theMsgR) :
	TMessage(theMsgR),
    mKeyBlockId( 0 ),
    mRawKeyP   ( NULL ),
    mRawKeyLen ( 0 ),
    mRawP      ( NULL ),
    mRawLen    ( 0 )
{
}


// Copy ctor
TMsgAuth1LoginBase2::TMsgAuth1LoginBase2(const TMsgAuth1LoginBase2& theMsgR) :
	TMessage(theMsgR),
	mKeyBlockId( theMsgR.mKeyBlockId ),
    mRawKey    ( theMsgR.mRawKey ),
    mRawKeyP   ( theMsgR.mRawKeyP ),
    mRawKeyLen ( theMsgR.mRawKeyLen ),
	mDataBlock ( theMsgR.mDataBlock ),
    mRawP      ( theMsgR.mRawP ),
    mRawLen    ( theMsgR.mRawLen )
{
}


// Destructor
TMsgAuth1LoginBase2::~TMsgAuth1LoginBase2(void)
{}


// ** Public Methods

// Assignment operator
TMsgAuth1LoginBase2&
TMsgAuth1LoginBase2::operator=(const TMsgAuth1LoginBase2& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
	    TMessage::operator=(theMsgR);
		mKeyBlockId    = theMsgR.mKeyBlockId;
        mRawKey        = theMsgR.mRawKey;
        mRawKeyP       = theMsgR.mRawKeyP;
        mRawKeyLen     = theMsgR.mRawKeyLen;

	    mDataBlock     = theMsgR.mDataBlock;
        mRawP          = theMsgR.mRawP;
        mRawLen        = theMsgR.mRawLen;
	}
	return *this;
}


// TMsgAuth1LoginBase2::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgAuth1LoginBase2::Pack(void)
{
	WTRACE("TMsgAuth1LoginBase2::Pack");
	TMessage::Pack();

	WDBG_LL("TMsgAuth1LoginBase2::Pack Appending message data");
	AppendShort(mKeyBlockId);
	AppendShort(mRawKeyLen);
	if (mRawKeyLen  > 0) AppendBytes(mRawKeyLen, mRawKeyP);
	if (mRawLen     > 0) AppendBytes(mRawLen,   mRawP);

	return GetDataPtr();
}


// TMsgAuth1LoginBase2::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
// Note: call ForceRawBufOwn() to force ownership of the data buffers.
void
TMsgAuth1LoginBase2::Unpack(void)
{
	WTRACE("TMsgAuth1LoginBase2::Unpack");
	TMessage::Unpack();

	WDBG_LL("TMsgAuth1LoginBase2::Unpack Reading message data");
	mKeyBlockId = ReadShort();
	mRawKeyLen  = ReadShort();
	if (mRawKeyLen == 0)
		mRawKeyP = NULL;
	else
		mRawKeyP = static_cast<const unsigned char*>(ReadBytes(mRawKeyLen));

    mRawLen = BytesLeftToRead();
	if (mRawLen == 0)
		mRawP = NULL;
	else
		mRawP = static_cast<const unsigned char*>(ReadBytes(mRawLen));
}

void
TMsgAuth1LoginBase2::SetRawKeyBuf(const unsigned char* theRawP, unsigned short theLen,
                                 bool copyBuf)
{
	WTRACE("TMsgAuth1LoginBase2::SetRawKeyBuf");
	if (! copyBuf)
	{
		WDBG_LL("TMsgAuth1LoginBase2::SetRawKeyBuf Assign buf (no copy!)");
		mRawKeyP   = theRawP; 
		mRawKeyLen = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuth1LoginBase2::SetRawKeyBuf Copy buf");
		mRawKeyLen = theLen;
		if ((mRawKeyLen > 0) && (theRawP))
		{
			mRawKey.assign(theRawP, mRawKeyLen);
			mRawKeyP = mRawKey.data();
		}
		else
			mRawKeyP = NULL;
	}
}

void
TMsgAuth1LoginBase2::SetRawDataBuf(const unsigned char* theRawP, unsigned short theLen,
                                 bool copyBuf)
{
	WTRACE("TMsgAuth1LoginBase2::SetRawDataBuf");
	if (! copyBuf)
	{
		WDBG_LL("TMsgAuth1LoginBase2::SetRawDataBuf Assign buf (no copy!)");
		mRawP   = theRawP; 
		mRawLen = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuth1LoginBase2::SetRawDataBuf Copy buf");
		mRawLen = theLen;
		if ((mRawLen > 0) && (theRawP))
		{
			mDataBlock.assign(theRawP, mRawLen);
			mRawP = mDataBlock.data();
		}
		else
			mRawP = NULL;
	}
}

