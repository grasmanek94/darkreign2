
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumeratePublications
//
// The EnumeratePublicationReply message contains a list of all of the publications on the
// Observation server published by the specified publisher.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsEnumPublications.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsEnumeratePublications;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumeratePublications::MMsgObsEnumeratePublications(void) :
	MiniMessage(),
	mPublisherId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublications);
}


// MiniMessage
MMsgObsEnumeratePublications::MMsgObsEnumeratePublications(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(0)
{
	Unpack();
}

// Copy
MMsgObsEnumeratePublications::MMsgObsEnumeratePublications(const MMsgObsEnumeratePublications& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId)
{
}

// Destructor
MMsgObsEnumeratePublications::~MMsgObsEnumeratePublications(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumeratePublications&
MMsgObsEnumeratePublications::operator=(const MMsgObsEnumeratePublications& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumeratePublications::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumeratePublications::Pack(void)
{
	WTRACE("MMsgObsEnumeratePublications::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublications);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumeratePublications::Pack Appending message data");

	AppendLong(mPublisherId);
		
	return GetDataPtr();
}


// MMsgObsEnumeratePublications::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumeratePublications::Unpack(void)
{
	WTRACE("MMsgObsEnumeratePublications::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumeratePublications))
	{
		WDBG_AH("MMsgObsEnumeratePublications::Unpack Not a ObsEnumeratePublications message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumeratePublications message.");
	}

	WDBG_LL("MMsgObsEnumeratePublications::Unpack Reading message data");

	mPublisherId = ReadLong();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumeratePublicationsReply
//
// The EnumeratePublicationsReply message sends a list of all of the publishers of this Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsEnumeratePublicationsReply;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumeratePublicationsReply::MMsgObsEnumeratePublicationsReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationList()
{	

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublicationsReply);
}


// MiniMessage
MMsgObsEnumeratePublicationsReply::MMsgObsEnumeratePublicationsReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationList()
{
	Unpack();
}

// Copy
MMsgObsEnumeratePublicationsReply::MMsgObsEnumeratePublicationsReply(const MMsgObsEnumeratePublicationsReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mPublicationList(theMsgR.mPublicationList)
{
}

// Destructor
MMsgObsEnumeratePublicationsReply::~MMsgObsEnumeratePublicationsReply(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumeratePublicationsReply&
MMsgObsEnumeratePublicationsReply::operator=(const MMsgObsEnumeratePublicationsReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus  = theMsgR.mStatus;
	mPublicationList = theMsgR.mPublicationList;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumeratePublicationsReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumeratePublicationsReply::Pack(void)
{
	WTRACE("MMsgObsEnumeratePublicationsReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumeratePublicationsReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumeratePublicationsReply::Pack Appending message data");

	AppendShort((short)(mStatus));
	AppendLong(mPublicationList.size()); // Number of publishers

	for (PublicationList::iterator aItr = mPublicationList.begin(); aItr != mPublicationList.end(); aItr++)
	{
		AppendPublication(*aItr);
	}
	
	return GetDataPtr();
}


// MMsgObsEnumeratePublicationsReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumeratePublicationsReply::Unpack(void)
{
	WTRACE("MMsgObsEnumeratePublicationsReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumeratePublicationsReply))
	{
		WDBG_AH("MMsgObsEnumeratePublicationsReply::Unpack Not a ObsEnumeratePublicationsReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumeratePublicationsReply message.");
	}

	WDBG_LL("MMsgObsEnumeratePublicationsReply::Unpack Reading message data");


	mStatus = (ServerStatus)(short)(ReadShort());
	unsigned long aPublicationListSize = ReadLong();

	// Clear current list
	mPublicationList.clear();

	for (int i=0; i < aPublicationListSize; i++)
	{
		ReadPublication();
	}
}


////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////

// Add Publication to message
void
MMsgObsEnumeratePublicationsReply::ReadPublication()
{
	WTRACE("MMsgObsEnumeratePublicationsReply::ReadPublication");

	Publication aPublication;

	aPublication.mId						= ReadLong();
	ReadString(aPublication.mName);
	aPublication.mPublisherId				= ReadLong();
	ReadWString(aPublication.mPublicationDescription);
	aPublication.mPublicationType			= (PublicationTypeEnum)ReadShort();
	aPublication.mSubscriptionNotification	= ReadShort() ? true : false;

	// Data pool data was not sent Data length will be zero and Data pointer will be NULL
	
	mPublicationList.push_back(aPublication);
}

// Get Publication from message
void
MMsgObsEnumeratePublicationsReply::AppendPublication(const Publication& thePublicationR)
{
	WTRACE("MMsgObsEnumeratePublicationsReply::AppendPublication");

	AppendLong(thePublicationR.mId);					
	Append_PA_STRING(thePublicationR.mName);
	AppendLong(thePublicationR.mPublisherId);
	Append_PW_STRING(thePublicationR.mPublicationDescription);
	AppendShort(thePublicationR.mPublicationType);
	AppendShort((short)thePublicationR.mSubscriptionNotification);

	// Don't send Data pool data here
}


