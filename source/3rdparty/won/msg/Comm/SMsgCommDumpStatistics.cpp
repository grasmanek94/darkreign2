#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesComm.h"
#include "SMsgCommDumpStatistics.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgCommDumpStatistics;
};

SMsgCommDumpStatistics::SMsgCommDumpStatistics()
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::SmallCommDumpStatistics);
}


SMsgCommDumpStatistics::~SMsgCommDumpStatistics(void)
{}


SMsgCommDumpStatistics::SMsgCommDumpStatistics(const SMsgCommDumpStatistics& theMsgR) :
    SmallMessage(theMsgR)
{}


SMsgCommDumpStatistics::SMsgCommDumpStatistics(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR)
{
	Unpack();
}


SMsgCommDumpStatistics& SMsgCommDumpStatistics::operator =(const SMsgCommDumpStatistics& theMsgR)
{
	if(this != &theMsgR)
		SmallMessage::operator=(theMsgR);

	return *this;
}


void* SMsgCommDumpStatistics::Pack(void)
{
	WTRACE("SMsgCommDumpStatistics::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::SmallCommDumpStatistics);
	SmallMessage::Pack();

	return GetDataPtr();
}


void SMsgCommDumpStatistics::Unpack(void)
{
	WTRACE("SMsgCommDumpStatistics::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallCommonService ||
	    GetMessageType() != WONMsg::SmallCommDumpStatistics)
	{
		WDBG_AH("SMsgCommDumpStatistics::Unpack Not a SMsgCommDumpStatistics message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a SMsgCommDumpStatistics message.");
	}
}
