// TMsgAuthLastRawBufferBase.h

// Base class for AuthServer that adds a raw (binary) buffer of arbitrary length
// to TMessage.  This class is not a AuthServer message itself.  It provides methods
// fetch, set, pack and unpack a the binary buffer.

// Note that this message does not implement Pack and Unpack as the raw buffer
// may appended anywhere within an AuthServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "TMsgAuthLastRawBufferBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONMsg::TMessage;
	using WONMsg::TMsgAuthLastRawBufferBase;
};


// ** Constructors / Destructor

// Default ctor
TMsgAuthLastRawBufferBase::TMsgAuthLastRawBufferBase() :
	TMessage(),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{}


// TMessage ctor
TMsgAuthLastRawBufferBase::TMsgAuthLastRawBufferBase(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	Unpack();
}


// Copy ctor
TMsgAuthLastRawBufferBase::TMsgAuthLastRawBufferBase(const TMsgAuthLastRawBufferBase& theMsgR) :
	TMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
}


// Destructor
TMsgAuthLastRawBufferBase::~TMsgAuthLastRawBufferBase()
{}


// ** Protected Methods **

// TMsgAuthLastRawBufferBase::PackRawBuf
// Packs 2 byte length of raw buffer and raw buffer.  
void
TMsgAuthLastRawBufferBase::PackRawBuf()
{
	WTRACE("TMsgAuthLastRawBufferBase::PackRawBuf");
	WDBG_LL("TMsgAuthLastRawBufferBase::Pack Appending RawBuf");
	if (mRawLen > 0) AppendBytes(mRawLen, mRawP);
}


// TMsgAuthRawBufferBase::UnpackRawBuf
// Unpacks raw buffer as 2 byte length followed by raw data.
void
TMsgAuthLastRawBufferBase::UnpackRawBuf(bool copyBuf)
{
	WTRACE("TMsgAuthLastRawBufferBase::UnpackRawBuf");
	WDBG_LL("TMsgAuthLastRawBufferBase::UnpackRawBuf Unpack raw buffer");
	mRawLen = BytesLeftToRead();

	if (mRawLen == 0)
		mRawP = NULL;
	else if (! copyBuf)
		mRawP = static_cast<const unsigned char*>(ReadBytes(mRawLen));
	else
	{
		mRawBuf.assign(static_cast<const unsigned char*>(ReadBytes(mRawLen)), mRawLen);
		mRawP = mRawBuf.data();
	}
}


// ** Public Methods **

// Assignment operator
TMsgAuthLastRawBufferBase&
TMsgAuthLastRawBufferBase::operator=(const TMsgAuthLastRawBufferBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
	}
	return *this;
}


void
TMsgAuthLastRawBufferBase::SetRawBuf(const unsigned char* theRawP, unsigned short theLen,
                                 bool copyBuf)
{
	WTRACE("TMsgAuthLastRawBufferBase::SetRawBuf");
	if (! copyBuf)
	{
		WDBG_LL("TMsgAuthLastRawBufferBase::SetRawBuf Assign buf (no copy!)");
		mRawP   = theRawP; 
		mRawLen = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuthLastRawBufferBase::SetRawBuf Copy buf");
		mRawLen = theLen;
		if ((mRawLen > 0) && (theRawP))
		{
			mRawBuf.assign(theRawP, mRawLen);
			mRawP = mRawBuf.data();
		}
		else
			mRawP = NULL;
	}
}
