
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumeratePublishers
//
// The EnumeratePublishers message requests a list of all of the publishers of this Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsEnumPublishers.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsEnumeratePublishers;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumeratePublishers::MMsgObsEnumeratePublishers(void) :
	MiniMessage()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublishers);
}


// MiniMessage
MMsgObsEnumeratePublishers::MMsgObsEnumeratePublishers(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	Unpack();
}

// Copy
MMsgObsEnumeratePublishers::MMsgObsEnumeratePublishers(const MMsgObsEnumeratePublishers& theMsgR) :
	MiniMessage(theMsgR)
{
}

// Destructor
MMsgObsEnumeratePublishers::~MMsgObsEnumeratePublishers(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumeratePublishers&
MMsgObsEnumeratePublishers::operator=(const MMsgObsEnumeratePublishers& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumeratePublishers::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumeratePublishers::Pack(void)
{
	WTRACE("MMsgObsEnumeratePublishers::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublishers);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumeratePublishers::Pack No message data to append");
	
	return GetDataPtr();
}


// MMsgObsEnumeratePublishers::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumeratePublishers::Unpack(void)
{
	WTRACE("MMsgObsEnumeratePublishers::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumeratePublishers))
	{
		WDBG_AH("MMsgObsEnumeratePublishers::Unpack Not an ObsEnumeratePublishers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsEnumeratePublishers message.");
	}

	WDBG_LL("MMsgObsEnumeratePublishers::Unpack No message data to read");
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumeratePublishersReply
//
// The EnumeratePublishersReply message sends a list of all of the publishers of this Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsEnumeratePublishersReply;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumeratePublishersReply::MMsgObsEnumeratePublishersReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mPublisherList()
{	

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublishersReply);
}


// MiniMessage
MMsgObsEnumeratePublishersReply::MMsgObsEnumeratePublishersReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mPublisherList()
{
	Unpack();
}

// Copy
MMsgObsEnumeratePublishersReply::MMsgObsEnumeratePublishersReply(const MMsgObsEnumeratePublishersReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mPublisherList(theMsgR.mPublisherList)
{
}

// Destructor
MMsgObsEnumeratePublishersReply::~MMsgObsEnumeratePublishersReply(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumeratePublishersReply&
MMsgObsEnumeratePublishersReply::operator=(const MMsgObsEnumeratePublishersReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus  = theMsgR.mStatus;
	mPublisherList = theMsgR.mPublisherList;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumeratePublishersReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumeratePublishersReply::Pack(void)
{
	WTRACE("MMsgObsEnumeratePublishersReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublishersReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumeratePublishersReply::Pack Appending message data");

	AppendShort((short)(mStatus));
	AppendLong(mPublisherList.size()); // Number of publishers

	for (PublisherList::iterator aItr = mPublisherList.begin(); aItr != mPublisherList.end(); aItr++)
	{
		AppendPublisher(*aItr);
	}
	
	return GetDataPtr();
}


// MMsgObsEnumeratePublishersReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumeratePublishersReply::Unpack(void)
{
	WTRACE("MMsgObsEnumeratePublishersReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumeratePublishersReply))
	{
		WDBG_AH("MMsgObsEnumeratePublishersReply::Unpack Not a ObsEnumeratePublishersReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumeratePublishersReply message.");
	}

	WDBG_LL("MMsgObsEnumeratePublishersReply::Unpack Reading message data");


	mStatus = (ServerStatus)(short)(ReadShort());
	unsigned long aPublisherListSize = ReadLong();

	// Clear current list
	mPublisherList.clear();

	for (int i=0; i < aPublisherListSize; i++)
	{
		ReadPublisher();
	}
}


////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////

// Add publisher to message
void
MMsgObsEnumeratePublishersReply::ReadPublisher()
{
	WTRACE("MMsgObsEnumeratePublishersReply::ReadPublisher");

	Publisher aPublisher;

	aPublisher.mId						= ReadLong();
	ReadString(aPublisher.mName);
	ReadWString(aPublisher.mPublisherDescription);
	aPublisher.mConnectionType			= (ConnectionTypeEnum)ReadShort();
	ReadString(aPublisher.mNetworkAddress);
	aPublisher.mAccessPubsByName		= ReadByte()?true:false;
	aPublisher.mOnDemandPublishing		= ReadByte()?true:false;

	mPublisherList.push_back(aPublisher);
}

// Get publisher from message
void
MMsgObsEnumeratePublishersReply::AppendPublisher(const Publisher& thePublisherR)
{
	WTRACE("MMsgObsEnumeratePublishersReply::AppendPublisher");

	AppendLong(thePublisherR.mId);					
	Append_PA_STRING(thePublisherR.mName);					
	Append_PW_STRING(thePublisherR.mPublisherDescription);
	AppendShort(thePublisherR.mConnectionType);		
	Append_PA_STRING(thePublisherR.mNetworkAddress);
	AppendByte((char)thePublisherR.mAccessPubsByName);
	AppendByte((char)thePublisherR.mOnDemandPublishing);
}

