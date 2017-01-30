#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingAcceptClient.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingAcceptClient;
	using WONMsg::MMsgRoutingAcceptClientReply;
};

//
// MMsgRoutingAcceptClient
//
MMsgRoutingAcceptClient::MMsgRoutingAcceptClient(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAcceptClient);
}

MMsgRoutingAcceptClient::~MMsgRoutingAcceptClient(void)
{}

MMsgRoutingAcceptClient::MMsgRoutingAcceptClient(const MMsgRoutingAcceptClient& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientName(theMsgR.mClientName)
{}

MMsgRoutingAcceptClient::MMsgRoutingAcceptClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingAcceptClient& MMsgRoutingAcceptClient::operator =(const MMsgRoutingAcceptClient& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mClientName = theMsgR.mClientName;
	}

	return *this;
}

void MMsgRoutingAcceptClient::Dump(ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  ClientName: " << mClientName << endl;
}

void* MMsgRoutingAcceptClient::Pack(void)
{
	WTRACE("MMsgRoutingAcceptClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAcceptClient);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingAcceptClient::Pack Appending message data");

	// append the client name
	AppendClientName(mClientName);

	return GetDataPtr();
}

void MMsgRoutingAcceptClient::Unpack(void)
{
	WTRACE("MMsgRoutingAcceptClient::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingAcceptClient)
	{
		WDBG_AH("MMsgRoutingAcceptClient::Unpack Not a RoutingAcceptClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingAcceptClient message.");
	}

	WDBG_LL("MMsgRoutingAcceptClient::Unpack Reading message data");

	// read in the client name
	ReadClientName(mClientName);
}

//
// MMsgRoutingAcceptClientReply
//
MMsgRoutingAcceptClientReply::MMsgRoutingAcceptClientReply(void) : 
	RoutingServerMessage(),
	mAcceptOrRejectFlag(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAcceptClientReply);
}

MMsgRoutingAcceptClientReply::~MMsgRoutingAcceptClientReply(void)
{}

MMsgRoutingAcceptClientReply::MMsgRoutingAcceptClientReply(const MMsgRoutingAcceptClientReply& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientName(theMsgR.mClientName),
	mAcceptOrRejectFlag(theMsgR.mAcceptOrRejectFlag),
	mComment(theMsgR.mComment)
{}

MMsgRoutingAcceptClientReply::MMsgRoutingAcceptClientReply(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingAcceptClientReply& MMsgRoutingAcceptClientReply::operator =(const MMsgRoutingAcceptClientReply& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientName           = theMsgR.mClientName;
		mAcceptOrRejectFlag = theMsgR.mAcceptOrRejectFlag;
		mComment            = theMsgR.mComment;
	}

	return *this;
}

void MMsgRoutingAcceptClientReply::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  ClientName: " << mClientName << "\n";
	os << "  Accept/Reject: " << (mAcceptOrRejectFlag ? "accept" : "reject") << "\n";
	os << "  Comment: " << mComment << "\n";
}

void* MMsgRoutingAcceptClientReply::Pack(void)
{
	WTRACE("MMsgRoutingAcceptClientReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAcceptClientReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingAcceptClientReply::Pack Appending message data");

	// append client name
	AppendClientName(mClientName);

	// append the accept/reject flag
	AppendBool(mAcceptOrRejectFlag);

	// append the comment
	Append_PW_STRING(mComment);

	return GetDataPtr();
}

void MMsgRoutingAcceptClientReply::Unpack(void)
{
	WTRACE("MMsgRoutingAcceptClientReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingAcceptClientReply)
	{
		WDBG_AH("MMsgRoutingAcceptClientReply::Unpack Not a RoutingAcceptClientReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingAcceptClientReply message.");
	}

	WDBG_LL("MMsgRoutingAcceptClientReply::Unpack Reading message data");

	// read in client name
	ReadClientName(mClientName);

	// read in the accept/reject flag
	mAcceptOrRejectFlag = ReadBool();

	// read in the comment
	mComment = Read_PW_STRING();
}