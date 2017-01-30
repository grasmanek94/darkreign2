// MMsgCommMRTG

// Common MRTG message class.  Supports MRTG requests and replies to
// WON servers.  


#include "common/won.h"
#include <limits.h>
#ifdef WIN32
#include <process.h>
#endif
#include <sys/stat.h>
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "msg/ServerStatus.h"
#include "MMsgTypesComm.h"
#include "MMsgCommMRTG.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommMRTGRequest;
	using WONMsg::MMsgCommMRTGReply;
};


// ** MMsgCommGetNetStat **

// ** Constructors / Destructor

// Default ctor
MMsgCommMRTGRequest::MMsgCommMRTGRequest(void) :
	MiniMessage(), mPrimaryID(0), mSecondaryID(0), 
	mPrimaryOption(), mSecondaryOption()
{
	WTRACE("MMsgCommMRTGRequest::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommMRTGRequest);
}


// MiniMessage ctor
MMsgCommMRTGRequest::MMsgCommMRTGRequest(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR), mPrimaryID(0), mSecondaryID(0), 
	mPrimaryOption(), mSecondaryOption()
{
	WTRACE("MMsgCommMRTGRequest::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommMRTGRequest::MMsgCommMRTGRequest(const MMsgCommMRTGRequest& theMsgR) :
	MiniMessage(theMsgR), 
	mPrimaryID(theMsgR.mPrimaryID), mSecondaryID(theMsgR.mSecondaryID), 
	mPrimaryOption(theMsgR.mPrimaryOption), mSecondaryOption(theMsgR.mSecondaryOption)
{
	WTRACE("MMsgCommMRTGRequest::ctor(copy)");
}


// Destructor
MMsgCommMRTGRequest::~MMsgCommMRTGRequest(void)
{
	WTRACE("MMsgCommMRTGRequest::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommMRTGRequest&
MMsgCommMRTGRequest::operator=(const MMsgCommMRTGRequest& theMsgR)
{
	WTRACE("MMsgCommMRTGRequest::operator=");
	MiniMessage::operator=(theMsgR);
	mPrimaryID = theMsgR.mPrimaryID;
	mSecondaryID = theMsgR.mSecondaryID;
	mPrimaryOption = theMsgR.mPrimaryOption;
	mSecondaryOption = theMsgR.mSecondaryOption;
	return *this;
}

// MMsgCommMRTGRequest::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommMRTGRequest::Pack(void)
{
	WTRACE("MMsgCommMRTGRequest::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommMRTGRequest);
	MiniMessage::Pack();
	AppendShort(mPrimaryID);
	AppendShort(mSecondaryID);
	AppendRawString(mPrimaryOption);
	AppendRawString(mSecondaryOption);
	return GetDataPtr();
}


// MMsgCommMRTGRequest::unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommMRTGRequest::Unpack(void)
{
	WTRACE("MMsgCommMRTGRequest::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommMRTGRequest))
	{
		WDBG_AH("MMsgCommMRTGRequest::Unpack Not a MiniCommMRTGRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommMRTGRequest message.");
	}
	WDBG_LL("MMsgCommMRTGRequest::Unpack Reading message data");
	mPrimaryID = ReadShort();
	mSecondaryID = ReadShort();
	ReadRawString(mPrimaryOption);
	ReadRawString(mSecondaryOption);
}


// ** MMsgCommMRTGReply **

// ** Constructors / Destructor

// Default ctor
MMsgCommMRTGReply::MMsgCommMRTGReply(void) :
	MiniMessage(),
		mPrimaryData(0), mSecondaryData(0), mStatus(WONMsg::StatusCommon_Success)
{
	WTRACE("MMsgCommMRTGReply::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommMRTGReply);
}


// TMessage ctor
MMsgCommMRTGReply::MMsgCommMRTGReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPrimaryData(0), mSecondaryData(0), mStatus(WONMsg::StatusCommon_Success)

{
	WTRACE("MMsgCommMRTGReply::ctor(TMessage)");
	Unpack();
}


// Copy ctor
MMsgCommMRTGReply::MMsgCommMRTGReply(const MMsgCommMRTGReply& theMsgR) :
	MiniMessage(theMsgR),
	mPrimaryData(theMsgR.mPrimaryData), mSecondaryData(theMsgR.mSecondaryData), 
	mStatus(theMsgR.mStatus)
{
	WTRACE("MMsgCommMRTGReply::ctor(copy)");
}


// Destructor
MMsgCommMRTGReply::~MMsgCommMRTGReply(void)
{
	WTRACE("MMsgCommMRTGReply::destruct");
}


// ** Public Methods **

// Assignment operator
MMsgCommMRTGReply&
MMsgCommMRTGReply::operator=(const MMsgCommMRTGReply& theMsgR)
{
	WTRACE("MMsgCommGetNetStat::operator=");
	MiniMessage::operator=(theMsgR);

	mPrimaryData = theMsgR.mPrimaryData;
	mSecondaryData = theMsgR.mSecondaryData;
	mStatus = theMsgR.mStatus;

	return *this;
}


// MMsgCommMRTGReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommMRTGReply::Pack(void)
{
	WTRACE("MMsgCommMRTGReply::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommMRTGReply);
	MiniMessage::Pack();

	AppendShort(mStatus);
	AppendLongLong(mPrimaryData);
	AppendLongLong(mSecondaryData);

	return GetDataPtr();
}


// MMsgCommMRTGReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgCommMRTGReply::Unpack(void)
{
	WTRACE("MMsgCommMRTGReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommMRTGReply))
	{
		WDBG_AH("MMsgCommMRTGReply::Unpack Not a MiniCommMRTGReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a MiniCommMRTGReply message.");
	}

	// Read startTick and determine lag
	WDBG_LL("MMsgCommMRTGReply::Unpack Reading message data");
	mStatus = ReadShort();
	mPrimaryData = ReadLongLong();
	mSecondaryData = ReadLongLong();
}
