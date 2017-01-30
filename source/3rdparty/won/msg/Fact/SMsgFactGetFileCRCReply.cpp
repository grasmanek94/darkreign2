

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetFileCRCReply.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgFactGetFileCRCReply::SMsgFactGetFileCRCReply(void) : SmallMessage(), mStatus(0), mCRC(0) 
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFileCRCReply);
}


SMsgFactGetFileCRCReply::~SMsgFactGetFileCRCReply(void)
{
}


SMsgFactGetFileCRCReply::SMsgFactGetFileCRCReply(const SMsgFactGetFileCRCReply& theMsgR) : SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus), mCRC(theMsgR.mCRC)
{
}


SMsgFactGetFileCRCReply::SMsgFactGetFileCRCReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mStatus(0), mCRC(0)
{
	Unpack();
}


SMsgFactGetFileCRCReply& SMsgFactGetFileCRCReply::operator =(const SMsgFactGetFileCRCReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mCRC = theMsgR.mCRC;

	return *this;

}


void* SMsgFactGetFileCRCReply::Pack(void)
{
	WTRACE("SMsgFactGetFileCRCReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFileCRCReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetFileCRCReply::Pack Appending message data");
	AppendShort(mStatus);
	AppendLong(mCRC);

	return GetDataPtr();
}


void SMsgFactGetFileCRCReply::Unpack(void)
{
	WTRACE("SMsgFactGetFileCRCReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetFileCRCReply))
	{
		WDBG_AH("SMsgFactGetFileCRCReply::Unpack Not a FactGetFileCRCReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactGetFileCRCReply message.");
	}

	WDBG_LL("SMsgFactGetFileCRCReply::Unpack Reading message data");
	mStatus = ReadShort();
	mCRC = ReadLong();
}
