// MMsgCommPing

// Common Ping message class.  Supports ping requests and replys to
// WON servers.  Ping reply may return extended info if requested.


#include "common/won.h"
#include <limits.h>
#ifdef WIN32
#include <process.h>
#endif
#include <sys/stat.h>
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesComm.h"
#include "MMsgCommPing.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommPing;
	using WONMsg::MMsgCommPingReply;
};


// ** MMsgCommPing **

// ** Constructors / Destructor

// Default ctor
MMsgCommPing::MMsgCommPing(bool getExtended) :
	MiniMessage(),
	mStartTick(0),
	mGetExtended(getExtended)
{
	WTRACE("MMsgCommPing::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommPing);
}


// MiniMessage ctor
MMsgCommPing::MMsgCommPing(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStartTick(0),
	mGetExtended(false)
{
	WTRACE("MMsgCommPing::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommPing::MMsgCommPing(const MMsgCommPing& theMsgR) :
	MiniMessage(theMsgR),
	mStartTick(theMsgR.mStartTick),
	mGetExtended(theMsgR.mGetExtended)
{
	WTRACE("MMsgCommPing::ctor(copy)");
}


// Destructor
MMsgCommPing::~MMsgCommPing(void)
{
	WTRACE("MMsgCommPing::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommPing&
MMsgCommPing::operator=(const MMsgCommPing& theMsgR)
{
	WTRACE("MMsgCommPing::operator=");
	MiniMessage::operator=(theMsgR);
	mStartTick   = theMsgR.mStartTick;
	mGetExtended = theMsgR.mGetExtended;
	return *this;
}


// MMsgCommPing::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommPing::Pack(void)
{
	WTRACE("MMsgCommPing::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommPing);
	MiniMessage::Pack();

	// Generate and pack in startTick
	WDBG_LL("MMsgCommPing::Pack Appending message data");
	AppendLong(GetTickCount());
	AppendByte(mGetExtended ? 1 : 0);

	return GetDataPtr();
}


// MMsgCommPing::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommPing::Unpack(void)
{
	WTRACE("MMsgCommPing::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommPing))
	{
		WDBG_AH("MMsgCommPing::Unpack Not a MiniCommPing message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommPing message.");
	}

	// Read attributes
	WDBG_LL("MMsgCommPing::Unpack Reading message data");
	mStartTick   = ReadLong();
	mGetExtended = (ReadByte() == 0 ? false : true);
}


// ** MMsgCommPingReply **

// ** Constructors / Destructor

// Default ctor
MMsgCommPingReply::MMsgCommPingReply(bool isExtended) :
	MiniMessage(),
	mStartTick(0),
	mLag(0),
	mExtended(isExtended),
	mAppName(),
	mLogicalName(),
	mImage(),
	mVersion(0),
	mPID(0),
	mPorts(),
	mIsAuth(false),
	mRegData(),
	mServData()
{
	WTRACE("MMsgCommPingReply::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommPingReply);
}


// TMessage ctor
MMsgCommPingReply::MMsgCommPingReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStartTick(0),
	mLag(0),
	mExtended(false),
	mAppName(),
	mLogicalName(),
	mImage(),
	mVersion(0),
	mPID(0),
	mPorts(),
	mIsAuth(false),
	mRegData(),
	mServData()

{
	WTRACE("MMsgCommPingReply::ctor(TMessage)");
	Unpack();
}


// Copy ctor
MMsgCommPingReply::MMsgCommPingReply(const MMsgCommPingReply& theMsgR) :
	MiniMessage(theMsgR),
	mStartTick(theMsgR.mStartTick),
	mLag(theMsgR.mLag),
	mExtended(theMsgR.mExtended),
	mAppName(theMsgR.mAppName),
	mLogicalName(theMsgR.mLogicalName),
	mImage(theMsgR.mImage),
	mVersion(theMsgR.mVersion),
	mPID(theMsgR.mPID),
	mPorts(theMsgR.mPorts),
	mIsAuth(theMsgR.mIsAuth),
	mRegData(theMsgR.mRegData),
	mServData(theMsgR.mServData)

{
	WTRACE("MMsgCommPingReply::ctor(copy)");
}


// Destructor
MMsgCommPingReply::~MMsgCommPingReply(void)
{
	WTRACE("MMsgCommPingReply::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommPingReply&
MMsgCommPingReply::operator=(const MMsgCommPingReply& theMsgR)
{
	WTRACE("MMsgCommPing::operator=");
	MiniMessage::operator=(theMsgR);

	mStartTick   = theMsgR.mStartTick;
	mLag         = theMsgR.mLag;
	mExtended    = theMsgR.mExtended;
	mAppName     = theMsgR.mAppName;
	mLogicalName = theMsgR.mLogicalName;
	mImage       = theMsgR.mImage;
	mVersion     = theMsgR.mVersion;
	mPID         = theMsgR.mPID;
	mPorts       = theMsgR.mPorts;
	mIsAuth      = theMsgR.mIsAuth;
	mRegData     = theMsgR.mRegData;
	mServData    = theMsgR.mServData;

	return *this;
}


// MMsgCommPingReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommPingReply::Pack(void)
{
	WTRACE("MMsgCommPingReply::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommPingReply);
	MiniMessage::Pack();

	// Add startTick
	AppendLong(mStartTick);

	// Only add other fields if extended
	if (mExtended)
	{
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

		WDBG_LL("MMsgCommPingReply::Pack Appending message data");
		Append_PA_STRING(mAppName);
		Append_PA_STRING(mLogicalName);
		AppendLong(mVersion);
		Append_PA_STRING(mImage);
		AppendLong(mPID);
		Append_PA_STRING(mPorts);
		AppendByte(mIsAuth ? 1 : 0);
		Append_PA_STRING(mRegData);
		Append_PA_STRING(mServData);
	}

	return GetDataPtr();
}


// MMsgCommPingReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommPingReply::Unpack(void)
{
	WTRACE("MMsgCommPingReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommPingReply))
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a MiniCommPingReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommPingReply message.");
	}

	// Read startTick and determine lag
	WDBG_LL("MMsgCommPingReply::Unpack Reading message data");
	unsigned long aTick = GetTickCount();
	mStartTick = ReadLong();
	mLag       = (aTick < mStartTick ? ULONG_MAX - mStartTick + aTick
	                                 : aTick - mStartTick);

	// See if there is more data, if so reply is extened.
	mExtended = (BytesLeftToRead() > 0);
	if (mExtended)
	{
		WDBG_LL("MMsgCommPingReply::Unpack Reading extended info");
		ReadString(mAppName);
		ReadString(mLogicalName);
		mVersion = ReadLong();
		ReadString(mImage);
		mPID = ReadLong();
		ReadString(mPorts);
		mIsAuth = (ReadByte() == 0 ? false : true);
		if (BytesLeftToRead() > 0)
		{
			ReadString(mRegData);
			ReadString(mServData);
		}
		else
		{
			mRegData.erase();
			mServData.erase();
		}
	}
	else
	{
		WDBG_LL("MMsgCommPingReply::Unpack No extended info");
		mAppName.erase();
		mLogicalName.erase();
		mVersion = 0;
		mImage.erase();
		mPID = 0;
		mPorts.erase();
		mIsAuth = false;
		mRegData.erase();
		mServData.erase();
	}
}
