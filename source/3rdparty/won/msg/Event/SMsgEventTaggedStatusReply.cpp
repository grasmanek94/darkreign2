#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesEvent.h"
#include "SMsgEventTaggedStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgEventStatusReply;
	using WONMsg::SMsgEventTaggedStatusReply;
};

SMsgEventTaggedStatusReply::SMsgEventTaggedStatusReply(void) : 
    SMsgEventStatusReply(),
	mTag(0)
{
	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventTaggedStatusReply);
}

SMsgEventTaggedStatusReply::~SMsgEventTaggedStatusReply(void)
{}

SMsgEventTaggedStatusReply::SMsgEventTaggedStatusReply(const SMsgEventTaggedStatusReply& theMsgR) :
    SMsgEventStatusReply(theMsgR),
	mTag(theMsgR.mTag)
{}

SMsgEventTaggedStatusReply::SMsgEventTaggedStatusReply(const SmallMessage& theMsgR) : 
    SMsgEventStatusReply(theMsgR, false)
{
	Unpack();
}

SMsgEventTaggedStatusReply& SMsgEventTaggedStatusReply::operator =(const SMsgEventTaggedStatusReply& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    // set fields from theMsgR
	mTag = theMsgR.mTag;

	return *this;
}

void* SMsgEventTaggedStatusReply::Pack(void)
{
	WTRACE("SMsgEventTaggedStatusReply::Pack");

	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventTaggedStatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgEventTaggedStatusReply::Pack Appending message data");

	AppendShort(mTag);
	AppendShort(mStatus);
	
	return GetDataPtr();
}

void SMsgEventTaggedStatusReply::Unpack(void)
{
	WTRACE("SMsgEventTaggedStatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallEventServer) ||
	    (GetMessageType() != WONMsg::EventTaggedStatusReply))
	{
		WDBG_AH("SMsgEventTaggedStatusReply::Unpack Not a EventTaggedStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a EventTaggedStatusReply message.");
	}

	WDBG_LL("SMsgEventTaggedStatusReply::Unpack Reading message data");

	mTag    = ReadShort();
	mStatus = ReadShort();
}

void SMsgEventTaggedStatusReply::Dump(std::ostream& os) const
{
	os << "  Tag: " << mTag << endl;
}