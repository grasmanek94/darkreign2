

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDB.h"
#include "SMsgDBVerifyUserReply.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgDBVerifyUserReply::SMsgDBVerifyUserReply(void) : SmallMessage(), mStatus(0)
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBVerifyUserReply);
}


SMsgDBVerifyUserReply::~SMsgDBVerifyUserReply(void)
{
}


SMsgDBVerifyUserReply::SMsgDBVerifyUserReply(const SMsgDBVerifyUserReply& theMsgR) : SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus), mResultText(theMsgR.mResultText)
{
}


SMsgDBVerifyUserReply::SMsgDBVerifyUserReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR), mStatus(0)
{
	Unpack();
}


SMsgDBVerifyUserReply& SMsgDBVerifyUserReply::operator =(const SMsgDBVerifyUserReply& theMsgR)
{

	if(this == &theMsgR)
		return *this;

	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mResultText = theMsgR.mResultText;
	return *this;

}


void* SMsgDBVerifyUserReply::Pack(void)
{
	WTRACE("SMsgDBVerifyUserReply::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBVerifyUserReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgDBVerifyUserReply::Pack Appending message data");
	AppendShort(mStatus);
	Append_PA_STRING(mResultText);
	return GetDataPtr();
}


void SMsgDBVerifyUserReply::Unpack(void)
{
	WTRACE("SMsgDBVerifyUserReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallCommonService) ||
	    (GetMessageType() != WONMsg::DBVerifyUserReply))
	{
		WDBG_AH("SMsgDBVerifyUserReply::Unpack Not a DBVerifyUserReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DBVerifyUserReply message.");
	}

	WDBG_LL("SMsgDBVerifyUserReply::Unpack Reading message data");
	mStatus = ReadShort();
	ReadString(mResultText);
}
