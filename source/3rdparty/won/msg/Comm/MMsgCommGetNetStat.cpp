// MMsgCommGetNetStat

// Common GetNetStat message class.  Supports GetNetStat requests and replys to
// WON servers.  GetNetStat reply may return extended info if requested.


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
#include "MMsgCommGetNetStat.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommGetNetStat;
	using WONMsg::MMsgCommGetNetStatReply;
};


// ** MMsgCommGetNetStat **

// ** Constructors / Destructor

// Default ctor
MMsgCommGetNetStat::MMsgCommGetNetStat(bool getExtended) :
	MiniMessage()
{
	WTRACE("MMsgCommGetNetStat::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNetStat);
}


// MiniMessage ctor
MMsgCommGetNetStat::MMsgCommGetNetStat(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommGetNetStat::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommGetNetStat::MMsgCommGetNetStat(const MMsgCommGetNetStat& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommGetNetStat::ctor(copy)");
}


// Destructor
MMsgCommGetNetStat::~MMsgCommGetNetStat(void)
{
	WTRACE("MMsgCommGetNetStat::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommGetNetStat&
MMsgCommGetNetStat::operator=(const MMsgCommGetNetStat& theMsgR)
{
	WTRACE("MMsgCommGetNetStat::operator=");
	MiniMessage::operator=(theMsgR);
	return *this;
}


// MMsgCommGetNetStat::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommGetNetStat::Pack(void)
{
	WTRACE("MMsgCommGetNetStat::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNetStat);
	MiniMessage::Pack();

	return GetDataPtr();
}


// MMsgCommGetNetStat::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommGetNetStat::Unpack(void)
{
	WTRACE("MMsgCommGetNetStat::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommGetNetStat))
	{
		WDBG_AH("MMsgCommGetNetStat::Unpack Not a MiniCommGetNetStat message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommGetNetStat message.");
	}
}


// ** MMsgCommGetNetStatReply **

// ** Constructors / Destructor

// Default ctor
MMsgCommGetNetStatReply::MMsgCommGetNetStatReply(bool isExtended) :
	MiniMessage(),
	mNumSockets(0)
{
	WTRACE("MMsgCommGetNetStatReply::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNetStatReply);
}


// TMessage ctor
MMsgCommGetNetStatReply::MMsgCommGetNetStatReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mNumSockets(0), mConnectToListenSocketCount(0)

{
	WTRACE("MMsgCommGetNetStatReply::ctor(TMessage)");
	Unpack();
}


// Copy ctor
MMsgCommGetNetStatReply::MMsgCommGetNetStatReply(const MMsgCommGetNetStatReply& theMsgR) :
	MiniMessage(theMsgR),
	mNumSockets(theMsgR.mNumSockets),
	mTCPListenSockets(theMsgR.mTCPListenSockets),
	mUDPListenSockets(theMsgR.mUDPListenSockets),
	mLastAcceptTimes(theMsgR.mLastAcceptTimes),
	mPendingAccepts(theMsgR.mPendingAccepts),
	mConnectToListenSocketCount(theMsgR.mConnectToListenSocketCount)
{
	WTRACE("MMsgCommGetNetStatReply::ctor(copy)");
}


// Destructor
MMsgCommGetNetStatReply::~MMsgCommGetNetStatReply(void)
{
	WTRACE("MMsgCommGetNetStatReply::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommGetNetStatReply&
MMsgCommGetNetStatReply::operator=(const MMsgCommGetNetStatReply& theMsgR)
{
	WTRACE("MMsgCommGetNetStat::operator=");
	MiniMessage::operator=(theMsgR);

	mNumSockets = theMsgR.mNumSockets;
	mTCPListenSockets = theMsgR.mTCPListenSockets;
	mUDPListenSockets = theMsgR.mUDPListenSockets;
	mLastAcceptTimes = theMsgR.mLastAcceptTimes;
	mPendingAccepts = theMsgR.mPendingAccepts;
	mConnectToListenSocketCount = theMsgR.mConnectToListenSocketCount;

	return *this;
}


// MMsgCommGetNetStatReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommGetNetStatReply::Pack(void)
{
	WTRACE("MMsgCommGetNetStatReply::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommGetNetStatReply);
	MiniMessage::Pack();

	AppendLong(mNumSockets);

	AppendShort(mTCPListenSockets.size());
	std::list<unsigned short>::iterator anItr = mTCPListenSockets.begin();
	while(anItr!=mTCPListenSockets.end())
	{
		AppendShort(*anItr);
		++anItr;
	}

	AppendShort(mUDPListenSockets.size());
	anItr = mUDPListenSockets.begin();
	while(anItr!=mUDPListenSockets.end())
	{
		AppendShort(*anItr);
		++anItr;
	}


	AppendLong(mConnectToListenSocketCount);
	AppendShort(mLastAcceptTimes.size());
	std::list<DWORD>::iterator anItr2;
	anItr2 = mLastAcceptTimes.begin();
	while(anItr2!=mLastAcceptTimes.end())
	{
		AppendLong(*anItr2);
		++anItr2;
	}

	AppendShort(mPendingAccepts.size());
	anItr2 = mPendingAccepts.begin();
	while(anItr2!=mPendingAccepts.end())
	{
		AppendLong(*anItr2);
		++anItr2;
	}

	return GetDataPtr();
}


// MMsgCommGetNetStatReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommGetNetStatReply::Unpack(void)
{
	WTRACE("MMsgCommGetNetStatReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommGetNetStatReply))
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a MiniCommGetNetStatReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommGetNetStatReply message.");
	}

	// Read startTick and determine lag
	WDBG_LL("MMsgCommGetNetStatReply::Unpack Reading message data");
	mNumSockets = ReadLong();
	int aNum = ReadShort();
	int i;

	mTCPListenSockets.clear();
	for(i=0; i<aNum; i++)
		mTCPListenSockets.push_back(ReadShort());

	aNum = ReadShort();
	mUDPListenSockets.clear();
	for(i=0; i<aNum; i++)
		mUDPListenSockets.push_back(ReadShort());

	if(BytesLeftToRead()<=0)
		return;

	mConnectToListenSocketCount = ReadLong();
	aNum = ReadShort();
	mLastAcceptTimes.clear();
	for(i=0; i<aNum; i++)
		mLastAcceptTimes.push_back(ReadLong());

	if(BytesLeftToRead()<=0)
		return;

	aNum = ReadShort();
	mPendingAccepts.clear();
	for(i=0; i<aNum; i++)
		mPendingAccepts.push_back(ReadLong());
}
