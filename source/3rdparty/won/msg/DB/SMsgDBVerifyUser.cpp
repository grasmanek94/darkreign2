

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDB.h"
#include "SMsgDBVerifyUser.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgDBVerifyUser::SMsgDBVerifyUser(void) : SmallMessage()
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBVerifyUser);
}


SMsgDBVerifyUser::~SMsgDBVerifyUser(void)
{
}


SMsgDBVerifyUser::SMsgDBVerifyUser(const SMsgDBVerifyUser& theMsgR) : SmallMessage(theMsgR),
	mUserName(theMsgR.mUserName)
{
}


SMsgDBVerifyUser::SMsgDBVerifyUser(const SmallMessage& theMsgR) : SmallMessage(theMsgR)
{
	Unpack();
}


SMsgDBVerifyUser& SMsgDBVerifyUser::operator =(const SMsgDBVerifyUser& theMsgR)
{

	if(this == &theMsgR)
		return *this;

	SmallMessage::operator=(theMsgR);
	mUserName = theMsgR.mUserName;
	return *this;

}


void* SMsgDBVerifyUser::Pack(void)
{
	WTRACE("SMsgDBVerifyUser::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBVerifyUser);
	SmallMessage::Pack();

	WDBG_LL("SMsgDBVerifyUser::Pack Appending message data");
	Append_PA_STRING(mUserName);
	return GetDataPtr();
}


void SMsgDBVerifyUser::Unpack(void)
{
	WTRACE("SMsgDBVerifyUser::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallCommonService) ||
	    (GetMessageType() != WONMsg::DBVerifyUser))
	{
		WDBG_AH("SMsgDBVerifyUser::Unpack Not a DBVerifyUser message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DBVerifyUser message.");
	}

	WDBG_LL("SMsgDBVerifyUser::Unpack Reading message data");
	ReadString(mUserName);
}
