#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesEvent.h"
#include "SMsgEventStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgEventStatusReply;
};

SMsgEventStatusReply::SMsgEventStatusReply(void) : 
    SmallMessage(),
	mStatus(0)
{
	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventStatusReply);
}

SMsgEventStatusReply::~SMsgEventStatusReply(void)
{}

SMsgEventStatusReply::SMsgEventStatusReply(const SMsgEventStatusReply& theMsgR) :
    SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{}

SMsgEventStatusReply::SMsgEventStatusReply(const SmallMessage& theMsgR, bool doUnpack) : 
    SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgEventStatusReply& SMsgEventStatusReply::operator =(const SMsgEventStatusReply& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    // set fields from theMsgR
	mStatus = theMsgR.mStatus;

	return *this;
}

void* SMsgEventStatusReply::Pack(void)
{
	WTRACE("SMsgEventStatusReply::Pack");

	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventStatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgEventStatusReply::Pack Appending message data");

	AppendShort(mStatus);		
	
	return GetDataPtr();
}

void SMsgEventStatusReply::Unpack(void)
{
	WTRACE("SMsgEventStatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallEventServer) ||
	    (GetMessageType() != WONMsg::EventStatusReply))
	{
		WDBG_AH("SMsgEventStatusReply::Unpack Not a EventStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a EventStatusReply message.");
	}

	WDBG_LL("SMsgEventStatusReply::Unpack Reading message data");

	mStatus = ReadShort();
}

void SMsgEventStatusReply::Dump(std::ostream& os) const
{
	os << "  Status: " << mStatus << endl;
}