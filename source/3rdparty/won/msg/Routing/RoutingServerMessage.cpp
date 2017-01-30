#include "RoutingServerMessage.h"

namespace {
	using WONCommon::RawBuffer;
	using WONMsg::MiniMessage;
	using WONMsg::RoutingServerMessage;
	using WONMsg::ClientName;
	using WONMsg::GroupName;
	using WONMsg::Password;
	using WONMsg::ClientId;
	using WONMsg::GroupId;
	using WONMsg::ClientOrGroupId;
};

RoutingServerMessage::RoutingServerMessage(void) : MiniMessage()
{}

RoutingServerMessage::~RoutingServerMessage(void)
{}

RoutingServerMessage::RoutingServerMessage(const RoutingServerMessage& theMsgR) : MiniMessage(theMsgR)
{}

RoutingServerMessage::RoutingServerMessage(const MiniMessage& theMsgR) : MiniMessage(theMsgR)
{}

RoutingServerMessage& RoutingServerMessage::operator=(const RoutingServerMessage& theMsgR)
{
	if (this != &theMsgR)
		MiniMessage::operator=(theMsgR);

	return *this;
}

void RoutingServerMessage::AppendClientName(const ClientName& theString)
{ AppendRawString(theString); }

void RoutingServerMessage::AppendGroupName(const GroupName& theString)
{ Append_PA_STRING(theString); }

void RoutingServerMessage::AppendPassword(const Password& theString)
{ Append_PW_STRING(theString); }

void RoutingServerMessage::AppendClientId(ClientId theClientId)
{ AppendShort(theClientId); }

void RoutingServerMessage::AppendGroupId(GroupId theGroupId)
{ AppendShort(theGroupId); }

void RoutingServerMessage::AppendClientOrGroupId(ClientOrGroupId theClientOrGroupId)
{ AppendShort(theClientOrGroupId); }

void RoutingServerMessage::ReadClientName(ClientName& theBufR) const
{ ReadRawString(theBufR); }

void RoutingServerMessage::ReadGroupName(GroupName& theBufR) const
{ ReadString(theBufR); }

void RoutingServerMessage::ReadPassword(Password& theBufR) const
{ ReadWString(theBufR); }

ClientId RoutingServerMessage::ReadClientId() const
{ return ReadShort(); }

GroupId RoutingServerMessage::ReadGroupId() const
{ return ReadShort(); }

ClientOrGroupId RoutingServerMessage::ReadClientOrGroupId() const
{ return ReadShort(); }
