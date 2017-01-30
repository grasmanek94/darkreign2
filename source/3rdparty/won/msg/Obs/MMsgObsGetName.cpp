
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetName
//
// The GetName message gets the unique name of the specified resource from the unique Identifier.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetName.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetName;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetName::MMsgObsGetName(void) :
	MiniMessage(),
	mResourceId(0),
	mResourceType(ResourcePublisher)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetName);
}


// MiniMessage
MMsgObsGetName::MMsgObsGetName(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mResourceId(0),
	mResourceType(ResourcePublisher)
{
	Unpack();
}

// Copy
MMsgObsGetName::MMsgObsGetName(const MMsgObsGetName& theMsgR) :
	MiniMessage(theMsgR),
	mResourceId(theMsgR.mResourceId),
	mResourceType(theMsgR.mResourceType)
{
}

// Destructor
MMsgObsGetName::~MMsgObsGetName(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetName&
MMsgObsGetName::operator=(const MMsgObsGetName& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mResourceId = theMsgR.mResourceId; 
	mResourceType = theMsgR.mResourceType;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetName::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetName::Pack(void)
{
	WTRACE("MMsgObsGetName::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetName);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetName::Pack Appending message data");

	AppendLong(mResourceId);
	AppendShort((short)mResourceType);
	
	return GetDataPtr();
}


// MMsgObsGetName::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetName::Unpack(void)
{
	WTRACE("MMsgObsGetName::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetName))
	{
		WDBG_AH("MMsgObsGetName::Unpack Not a ObsGetName message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetName message.");
	}

	WDBG_LL("MMsgObsGetName::Unpack Reading message data");

	mResourceId = ReadLong();
	mResourceType = (ResourceTypeEnum)ReadShort();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNameReply
//
// The GetNameReply message is sent to the client or server that requested the name of the specified
// resource.  It contains status of the operation as well as the name of the resource specified by the
// ResourceId in the GetName message.  
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetNameReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsGetNameReply::MMsgObsGetNameReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mResourceName(),
	mResourceDescription()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNameReply);
}


// MiniMessage 
MMsgObsGetNameReply::MMsgObsGetNameReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mResourceName(),
	mResourceDescription()
{
	Unpack();
}


// Copy 
MMsgObsGetNameReply::MMsgObsGetNameReply(const MMsgObsGetNameReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mResourceName(theMsgR.mResourceName),
	mResourceDescription(theMsgR.mResourceDescription)
{
}


// Destructor
MMsgObsGetNameReply::~MMsgObsGetNameReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetNameReply&
MMsgObsGetNameReply::operator=(const MMsgObsGetNameReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mResourceName = theMsgR.mResourceName;
	mResourceDescription = theMsgR.mResourceDescription;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetNameReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetNameReply::Pack(void)
{
	WTRACE("MMsgObsGetNameReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNameReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetNameReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	Append_PA_STRING(mResourceName);
	Append_PW_STRING(mResourceDescription);

	return GetDataPtr();
}


// MMsgObsGetNameReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetNameReply::Unpack(void)
{
	WTRACE("MMsgObsGetNameReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetNameReply))
	{
		WDBG_AH("MMsgObsGetNameReply::Unpack Not an ObsGetNameReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsGetNameReply message.");
	}

	WDBG_LL("MMsgObsGetNameReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	ReadString(mResourceName);
	ReadWString(mResourceDescription);
}


