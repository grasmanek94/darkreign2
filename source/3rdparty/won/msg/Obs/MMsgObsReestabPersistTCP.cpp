
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsReestablishPersistentTCP
//
// The ReestablishPersistentTCP message reestablishes a persistent TCP connection to the Observation server
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsReestabPersistTCP.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsReestablishPersistentTCP;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsReestablishPersistentTCP::MMsgObsReestablishPersistentTCP(void) :
	MiniMessage(),
	mClientId(0),
	mClientType(ResourcePublisher)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_ReestablishPersistentTCP);
}


// MiniMessage
MMsgObsReestablishPersistentTCP::MMsgObsReestablishPersistentTCP(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mClientId(0),
	mClientType(ResourcePublisher)
{
	Unpack();
}

// Copy
MMsgObsReestablishPersistentTCP::MMsgObsReestablishPersistentTCP(const MMsgObsReestablishPersistentTCP& theMsgR) :
	MiniMessage(theMsgR),
	mClientId(theMsgR.mClientId),
	mClientType(theMsgR.mClientType)
{
}

// Destructor
MMsgObsReestablishPersistentTCP::~MMsgObsReestablishPersistentTCP(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsReestablishPersistentTCP&
MMsgObsReestablishPersistentTCP::operator=(const MMsgObsReestablishPersistentTCP& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mClientId = theMsgR.mClientId; 
	mClientType = theMsgR.mClientType;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsReestablishPersistentTCP::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsReestablishPersistentTCP::Pack(void)
{
	WTRACE("MMsgObsReestablishPersistentTCP::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_ReestablishPersistentTCP);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsReestablishPersistentTCP::Pack Appending message data");

	AppendLong(mClientId);
	AppendShort((short)mClientType);
	
	return GetDataPtr();
}


// MMsgObsReestablishPersistentTCP::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsReestablishPersistentTCP::Unpack(void)
{
	WTRACE("MMsgObsReestablishPersistentTCP::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_ReestablishPersistentTCP))
	{
		WDBG_AH("MMsgObsReestablishPersistentTCP::Unpack Not a ObsReestablishPersistentTCP message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsReestablishPersistentTCP message.");
	}

	WDBG_LL("MMsgObsReestablishPersistentTCP::Unpack Reading message data");

	mClientId = ReadLong();
	mClientType = (ResourceTypeEnum)ReadShort();
}



