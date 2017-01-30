// TMsgCommPing.cpp

// Common Ping message classes.  Supports ping requests and replys to
// WON servers.


#include "common/won.h"
#include <limits.h>
#ifdef WIN32
#include <process.h>
#endif
#include <sys/stat.h>
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommPing.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommPing;
	using WONMsg::TMsgCommPingReply;
};


// ** TMsgCommPing **

// ** Constructors / Destructor

// Default ctor
TMsgCommPing::TMsgCommPing(void) :
	TMessage(),
	mStartTick(0)
{
	WTRACE("TMsgCommPing::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommPing);
}


// TMessage ctor
TMsgCommPing::TMsgCommPing(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStartTick(0)
{
	WTRACE("TMsgCommPing::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommPing::TMsgCommPing(const TMsgCommPing& theMsgR) :
	TMessage(theMsgR),
	mStartTick(theMsgR.mStartTick)
{
	WTRACE("TMsgCommPing::ctor(copy)");
}


// Destructor
TMsgCommPing::~TMsgCommPing(void)
{
	WTRACE("TMsgCommPing::destruct");
}


// ** Public Methods **

// Assignment operator
TMsgCommPing&
TMsgCommPing::operator=(const TMsgCommPing& theMsgR)
{
	WTRACE("TMsgCommPing::operator=");
	TMessage::operator=(theMsgR);
	mStartTick = theMsgR.mStartTick;
	return *this;
}


// TMsgCommPing::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommPing::Pack(void)
{
	WTRACE("TMsgCommPing::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommPing);
	TMessage::Pack();

	// Generate and pack in startTick
	WDBG_LL("TMsgCommPing::Pack Appending message data");
	AppendLong(GetTickCount());

	return GetDataPtr();
}


// TMsgCommPing::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommPing::Unpack(void)
{
	WTRACE("TMsgCommPing::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommPing))
	{
		WDBG_AH("TMsgCommPing::Unpack Not a CommPing message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommPing message.");
	}

	// Read attributes
	WDBG_LL("TMsgCommPing::Unpack Reading message data");
	mStartTick = ReadLong();
}


// ** TMsgCommPingReply **

// ** Constructors / Destructor

// Default ctor
TMsgCommPingReply::TMsgCommPingReply(void) :
	TMessage(),
	mAppName(),
	mLogicalName(),
	mImage(),
	mVersion(0),
	mPID(0),
	mPorts(),
	mStartTick(0),
	mLag(0)
{
	WTRACE("TMsgCommPingReply::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommPingReply);
}


// TMessage ctor
TMsgCommPingReply::TMsgCommPingReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mAppName(),
	mLogicalName(),
	mImage(),
	mVersion(0),
	mPID(0),
	mPorts(),
	mStartTick(0),
	mLag(0)
{
	WTRACE("TMsgCommPingReply::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommPingReply::TMsgCommPingReply(const TMsgCommPingReply& theMsgR) :
	TMessage(theMsgR),
	mAppName(theMsgR.mAppName),
	mLogicalName(theMsgR.mLogicalName),
	mImage(theMsgR.mImage),
	mVersion(theMsgR.mVersion),
	mPID(theMsgR.mPID),
	mPorts(theMsgR.mPorts),
	mStartTick(theMsgR.mStartTick),
	mLag(theMsgR.mLag)
{
	WTRACE("TMsgCommPingReply::ctor(copy)");
}


// Destructor
TMsgCommPingReply::~TMsgCommPingReply(void)
{
	WTRACE("TMsgCommPingReply::destruct");
}


// ** Public Methods **

// Assignment operator
TMsgCommPingReply&
TMsgCommPingReply::operator=(const TMsgCommPingReply& theMsgR)
{
	WTRACE("TMsgCommPing::operator=");
	TMessage::operator=(theMsgR);

	mAppName     = theMsgR.mAppName;
	mLogicalName = theMsgR.mLogicalName;
	mImage       = theMsgR.mImage;
	mVersion     = theMsgR.mVersion;
	mPID         = theMsgR.mPID;
	mPorts       = theMsgR.mPorts;
	mStartTick   = theMsgR.mStartTick;
	mLag         = theMsgR.mLag;

	return *this;
}


// TMsgCommPingReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommPingReply::Pack(void)
{
	WTRACE("TMsgCommPingReply::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommPingReply);
	TMessage::Pack();

#ifdef WIN32
	// Build image if needed
	if (mImage.empty())
	{
		char aFileNameStr[_MAX_PATH+1];
		GetModuleFileName(NULL, aFileNameStr, _MAX_PATH+1);
		mImage = aFileNameStr;
	}

	// Fetch version if needed
	if (mVersion == 0)
	{
		struct _stat aStat;
		mVersion = (_stat(mImage.c_str(), &aStat) == -1 ? 0 : aStat.st_mtime);
	}
#endif

#if defined(macintosh) && (macintosh == 1)
#else
		// Fetch PID if needed
		if (mPID == 0)
			mPID = getpid();
#endif

	WDBG_LL("TMsgCommPingReply::Pack Appending message data");
	Append_PA_STRING(mAppName);
	Append_PA_STRING(mLogicalName);
	AppendLong(mStartTick);
	AppendLong(mVersion);
	Append_PA_STRING(mImage);
	AppendLong(mPID);
	Append_PA_STRING(mPorts);

	return GetDataPtr();
}


// TMsgCommPingReply::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommPingReply::Unpack(void)
{
	WTRACE("TMsgCommPingReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommPingReply))
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a CommPingReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommPingReply message.");
	}

	WDBG_LL("TMsgCommPingReply::Unpack Reading message data");
	ReadString(mAppName);
	ReadString(mLogicalName);
	mStartTick = ReadLong();
	mVersion   = ReadLong();

	// If there's more to read, get extended info
	if (BytesLeftToRead() > 0)
	{
		WDBG_LL("TMsgCommPingReply::Unpack Reading extended info");
		ReadString(mImage);
		mPID = ReadLong();
		ReadString(mPorts);
	}
	else
	{
		WDBG_LL("TMsgCommPingReply::Unpack Extended info not present");
		mImage.erase();
		mPID = 0;
		mPorts.erase();
	}

	// Determine the lag
	unsigned long aTick = GetTickCount();
	mLag = (aTick < mStartTick ? ULONG_MAX - mStartTick + aTick : aTick - mStartTick);
}
