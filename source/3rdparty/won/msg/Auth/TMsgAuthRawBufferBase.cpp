// TMsgAuthRawBufferBase.h

// Base class for AuthServer that adds a raw (binary) buffer of arbitrary length
// to TMessage.  This class is not a AuthServer message itself.  It provides methods
// fetch, set, pack and unpack a the binary buffer.

// Note that this message does not implement Pack and Unpack as the raw buffer
// may appended anywhere within an AuthServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "TMsgAuthRawBufferBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONMsg::TMessage;
	using WONMsg::TMsgAuthRawBufferBase;
};


// ** Constructors / Destructor

// Default ctor
TMsgAuthRawBufferBase::TMsgAuthRawBufferBase() :
	TMessage(),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{}


// TMessage ctor
TMsgAuthRawBufferBase::TMsgAuthRawBufferBase(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	Unpack();
}


// Copy ctor
TMsgAuthRawBufferBase::TMsgAuthRawBufferBase(const TMsgAuthRawBufferBase& theMsgR) :
	TMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
}


// Destructor
TMsgAuthRawBufferBase::~TMsgAuthRawBufferBase()
{}


// ** Protected Methods **

// TMsgAuthRawBufferBase::PackRawBuf
// Packs 2 byte length of raw buffer and raw buffer.  
void
TMsgAuthRawBufferBase::PackRawBuf()
{
	WTRACE("TMsgAuthRawBufferBase::PackRawBuf");
	WDBG_LL("TMsgAuthRawBufferBase::Pack Appending RawBuf");
	AppendShort(mRawLen);
	if (mRawLen > 0) AppendBytes(mRawLen, mRawP);
}


// TMsgAuthRawBufferBase::UnpackRawBuf
// Unpacks raw buffer as 2 byte length followed by raw data.
void
TMsgAuthRawBufferBase::UnpackRawBuf(bool copyBuf)
{
	WTRACE("TMsgAuthRawBufferBase::UnpackRawBuf");
	WDBG_LL("TMsgAuthRawBufferBase::UnpackRawBuf Unpack raw buffer");
	mRawLen = ReadShort();

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
TMsgAuthRawBufferBase&
TMsgAuthRawBufferBase::operator=(const TMsgAuthRawBufferBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
	}
	return *this;
}


void
TMsgAuthRawBufferBase::SetRawBuf(const unsigned char* theRawP, unsigned short theLen,
                                 bool copyBuf)
{
	WTRACE("TMsgAuthRawBufferBase::SetRawBuf");
	if (! copyBuf)
	{
		WDBG_LL("TMsgAuthRawBufferBase::SetRawBuf Assign buf (no copy!)");
		mRawP   = theRawP; 
		mRawLen = theLen;
	}
	else
	{
		WDBG_LL("TMsgAuthRawBufferBase::SetRawBuf Copy buf");
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
