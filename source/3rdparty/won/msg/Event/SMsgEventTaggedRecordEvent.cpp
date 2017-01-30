#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesEvent.h"
#include "SMsgEventTaggedRecordEvent.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgEventRecordEvent;
	using WONMsg::SMsgEventTaggedRecordEvent;
};

SMsgEventTaggedRecordEvent::SMsgEventTaggedRecordEvent(void) : 
    SMsgEventRecordEvent(),
	mTag(0)
{
	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventTaggedRecordEvent);
}

SMsgEventTaggedRecordEvent::~SMsgEventTaggedRecordEvent(void)
{}

SMsgEventTaggedRecordEvent::SMsgEventTaggedRecordEvent(const SMsgEventTaggedRecordEvent& theMsgR) :
    SMsgEventRecordEvent(theMsgR),
	mTag(theMsgR.mTag)
{}

SMsgEventTaggedRecordEvent::SMsgEventTaggedRecordEvent(const SmallMessage& theMsgR) : 
    SMsgEventRecordEvent(theMsgR, false)
{
	Unpack();
}

SMsgEventTaggedRecordEvent& SMsgEventTaggedRecordEvent::operator =(const SMsgEventTaggedRecordEvent& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    // set fields from theMsgR
	SMsgEventRecordEvent::operator=(theMsgR);
	mTag = theMsgR.mTag;

	return *this;
}

void* SMsgEventTaggedRecordEvent::Pack(void)
{
	WTRACE("SMsgEventTaggedRecordEvent::Pack");

	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventTaggedRecordEvent);
	SmallMessage::Pack();

	WDBG_LL("SMsgEventTaggedRecordEvent::Pack Appending message data");

	// header-type stuff
	AppendShort(mTag);
	SimplePack();

	return GetDataPtr();
}

void SMsgEventTaggedRecordEvent::Unpack(void)
{
	WTRACE("SMsgEventTaggedRecordEvent::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallEventServer) ||
	    (GetMessageType() != WONMsg::EventTaggedRecordEvent))
	{
		WDBG_AH("SMsgEventTaggedRecordEvent::Unpack Not a EventTaggedRecordEvent message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a EventTaggedRecordEvent message.");
	}

	WDBG_LL("SMsgEventTaggedRecordEvent::Unpack Reading message data");

	// header-type stuff
	mTag = ReadShort();
	SimpleUnpack();
}

void SMsgEventTaggedRecordEvent::Dump(std::ostream& os) const
{
	os << "  Tag: " << mTag << endl;
	SMsgEventRecordEvent::Dump(os);
}