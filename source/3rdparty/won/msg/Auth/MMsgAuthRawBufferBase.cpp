// MMsgAuthRawBufferBase.h

// Base class for AuthServer that adds a raw (binary) buffer of arbitrary length
// to MiniMessage.  This class is not a AuthServer message itself.  It provides methods
// fetch, set, pack and unpack a the binary buffer.

// Note that this message does not implement Pack and Unpack as the raw buffer
// may appended anywhere within an AuthServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "MMsgAuthRawBufferBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::RawBuffer;
	using WONMsg::MiniMessage;
	using WONMsg::MMsgAuthRawBufferBase;
};


// ** Constructors / Destructor

// Default ctor
MMsgAuthRawBufferBase::MMsgAuthRawBufferBase() :
	MiniMessage(),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{}


// MiniMessage ctor
MMsgAuthRawBufferBase::MMsgAuthRawBufferBase(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	Unpack();
}


// Copy ctor
MMsgAuthRawBufferBase::MMsgAuthRawBufferBase(const MMsgAuthRawBufferBase& theMsgR) :
	MiniMessage(theMsgR),
	mRawP(NULL),
	mRawLen(0),
	mRawBuf()
{
	SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
}


// Destructor
MMsgAuthRawBufferBase::~MMsgAuthRawBufferBase()
{}


// ** Protected Methods **

// MMsgAuthRawBufferBase::PackRawBuf
// Packs 2 byte length of raw buffer and raw buffer.  
void
MMsgAuthRawBufferBase::PackRawBuf()
{
	WTRACE("MMsgAuthRawBufferBase::PackRawBuf");
	WDBG_LL("MMsgAuthRawBufferBase::Pack Appending RawBuf");
	AppendShort(mRawLen);
	if (mRawLen > 0) AppendBytes(mRawLen, mRawP);
}


// MMsgAuthRawBufferBase::UnpackRawBuf
// Unpacks raw buffer as 2 byte length followed by raw data.
void
MMsgAuthRawBufferBase::UnpackRawBuf(bool copyBuf)
{
	WTRACE("MMsgAuthRawBufferBase::UnpackRawBuf");
	WDBG_LL("MMsgAuthRawBufferBase::UnpackRawBuf Unpack raw buffer");
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
MMsgAuthRawBufferBase&
MMsgAuthRawBufferBase::operator=(const MMsgAuthRawBufferBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		MiniMessage::operator=(theMsgR);
		SetRawBuf(theMsgR.mRawP, theMsgR.mRawLen, true);
	}
	return *this;
}


void
MMsgAuthRawBufferBase::SetRawBuf(const unsigned char* theRawP, unsigned short theLen,
                                 bool copyBuf)
{
	WTRACE("MMsgAuthRawBufferBase::SetRawBuf");
	if (! copyBuf)
	{
		WDBG_LL("MMsgAuthRawBufferBase::SetRawBuf Assign buf (no copy!)");
		mRawP   = theRawP; 
		mRawLen = theLen;
	}
	else
	{
		WDBG_LL("MMsgAuthRawBufferBase::SetRawBuf Copy buf");
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
