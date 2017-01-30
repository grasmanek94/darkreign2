#pragma warning (disable : 4786)
#include "RoutingAPI.h"
#include "RoutingMessages.h"

using namespace WONAPI;

RoutingServerClient::ChatMessageBase::ChatMessageBase(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR) : 
	mSenderId(thePeerChatMsgR.GetClientId()), 
	mChatType(thePeerChatMsgR.GetChatType()),
	mIncludeExcludeFlag(thePeerChatMsgR.GetIncludeExcludeFlag())
{
	mRecipientIds.resize(thePeerChatMsgR.GetAddresseeList().size());
	std::copy<WONMsg::AddresseeList::IdList::const_iterator, std::vector<ClientOrGroupId>::iterator>(thePeerChatMsgR.GetAddresseeList().begin(), thePeerChatMsgR.GetAddresseeList().end(), mRecipientIds.begin());
}

RoutingServerClient::RawChatMessage::RawChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR) : 
	ChatMessageBase(thePeerChatMsgR), 
	mBufP(thePeerChatMsgR.GetData().data()), 
	mBufLen(thePeerChatMsgR.GetData().size()) 
{}

RoutingServerClient::ASCIIChatMessage::ASCIIChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR) : 
	ChatMessageBase(thePeerChatMsgR), 
	mData(reinterpret_cast<const char*>(thePeerChatMsgR.GetData().data()))
{}

RoutingServerClient::UnicodeChatMessage::UnicodeChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR) : 
	ChatMessageBase(thePeerChatMsgR)
{
	static wchar_t* aBufP = NULL;
	static unsigned int aBufLen = 0;
	unsigned int aNumChars = thePeerChatMsgR.GetData().size() / 2;
	if (aBufLen < aNumChars)
	{
		aBufLen = aNumChars * 2;
		delete aBufP;
		aBufP = new wchar_t[aBufLen];
	}
	WONCommon::ttohUnicodeString(reinterpret_cast<const wchar*>(thePeerChatMsgR.GetData().data()), aBufP, aNumChars);
	mData.assign(aBufP, aNumChars);
}

RoutingServerClient::CreateGroupResult::CreateGroupResult(const WONMsg::MMsgRoutingCreateGroupReply& theReplyR) :
	mStatus(theReplyR.GetStatus()),
	mGroupId(theReplyR.GetGroupId())
{}

RoutingServerClient::ClientDataResultBase::ClientDataResultBase(const WONMsg::MMsgRoutingGetClientListReply& theReplyR) :
	mStatus(theReplyR.GetStatus()),
	mIncludesIPs(theReplyR.IncludesIPs()),
	mIncludesAuthInfo(theReplyR.IncludesAuthInfo()),
	mIncludesMuteFlag(theReplyR.IncludesMuteFlag()),
	mIncludesModeratorFlag(theReplyR.IncludesModeratorFlag())
{}

RoutingServerClient::GetClientInfoResult::GetClientInfoResult(const WONMsg::MMsgRoutingGetClientListReply& theReplyR) :
	ClientDataResultBase(theReplyR)
{
	if (theReplyR.GetClientList().size())
		mClientData = (const WONMsg::MMsgRoutingGetClientListReply::ClientData&)*theReplyR.GetClientList().begin();
}

RoutingServerClient::GetClientListResult::GetClientListResult(const WONMsg::MMsgRoutingGetClientListReply& theReplyR) :
	ClientDataResultBase(theReplyR),
	mNumSpectators(theReplyR.GetNumSpectators()),
	mClientList((const std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>&)theReplyR.GetClientList())
{}

RoutingServerClient::GetGroupListResult::GetGroupListResult(const WONMsg::MMsgRoutingGetGroupListReply& theReplyR) :
	mStatus(theReplyR.GetStatus()),
	mGroupList((const std::list<GroupData>&)theReplyR.GetGroupList())
{}

	RoutingServerClient::GetSimpleClientListResult::GetSimpleClientListResult(const WONMsg::MMsgRoutingGetSimpleClientListReply& theReplyR) :
	mStatus(theReplyR.GetStatus()),
	mClientIdList(theReplyR.GetClientIdList()),
	mClientNameList(theReplyR.GetClientNameList()),
	mWONUserIdList(theReplyR.GetWONUserIdList())
{}

RoutingServerClient::GetNumUsersResult::GetNumUsersResult(const WONMsg::MMsgCommGetNumUsersReply& theReplyR) :
	mNumActiveUsers(theReplyR.GetNumActiveUsers()),
	mServerCapacity(theReplyR.GetUserCapacity())
{}

RoutingServerClient::ReadDataObjectResult::ReadDataObjectResult(const WONMsg::MMsgRoutingReadDataObjectReply& theReplyR) :
	mStatus(theReplyR.GetStatus())
{
	WONMsg::MMsgRoutingReadDataObjectReply::DataObjectSet::const_iterator itr = theReplyR.GetDataObjects().begin();
	for (; itr != theReplyR.GetDataObjects().end(); ++itr)
		mDataObjectList.push_back(DataObject(itr->mLinkId, itr->mOwnerId, itr->GetDataType(), itr->GetData()));
}

RoutingServerClient::RegisterClientResult::RegisterClientResult(const WONMsg::MMsgRoutingRegisterClientReply& theReplyR) : 
	mStatus(theReplyR.GetStatus()), 
	mHostName(theReplyR.GetHostName()), 
	mHostComment(theReplyR.GetHostComment()), 
	mClientId(theReplyR.GetClientId())
{}
