

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDB.h"
#include "SMsgDBRegisterUserReply.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgDBRegisterUserReply::SMsgDBRegisterUserReply(void) : SmallMessage(), mStatus(0)
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBRegisterUserReply);
}


SMsgDBRegisterUserReply::~SMsgDBRegisterUserReply(void)
{
}


SMsgDBRegisterUserReply::SMsgDBRegisterUserReply(const SMsgDBRegisterUserReply& theMsgR) : SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus), mResultText(theMsgR.mResultText)
{
}


SMsgDBRegisterUserReply::SMsgDBRegisterUserReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR), mStatus(0)
{
	Unpack();
}


SMsgDBRegisterUserReply& SMsgDBRegisterUserReply::operator =(const SMsgDBRegisterUserReply& theMsgR)
{

	if(this == &theMsgR)
		return *this;

	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mResultText = theMsgR.mResultText;
	return *this;

}


void* SMsgDBRegisterUserReply::Pack(void)
{
	WTRACE("SMsgDBRegisterUserReply::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBRegisterUserReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgDBRegisterUserReply::Pack Appending message data");
	AppendShort(mStatus);
	Append_PA_STRING(mResultText);
	return GetDataPtr();
}


void SMsgDBRegisterUserReply::Unpack(void)
{
	WTRACE("SMsgDBRegisterUserReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallCommonService) ||
	    (GetMessageType() != WONMsg::DBRegisterUserReply))
	{
		WDBG_AH("SMsgDBRegisterUserReply::Unpack Not a DBRegisterUserReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DBRegisterUserReply message.");
	}

	WDBG_LL("SMsgDBRegisterUserReply::Unpack Reading message data");
	mStatus = ReadShort();
	ReadString(mResultText);
}
