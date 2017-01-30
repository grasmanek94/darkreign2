#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingBaseDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBaseDataObject;
};

MMsgRoutingBaseDataObject::MMsgRoutingBaseDataObject(void) : 
	RoutingServerMessage(),
	mLinkId(0)
{}

MMsgRoutingBaseDataObject::~MMsgRoutingBaseDataObject(void)
{}

MMsgRoutingBaseDataObject::MMsgRoutingBaseDataObject(const MMsgRoutingBaseDataObject& theMsgR) :
    RoutingServerMessage(theMsgR),
	mLinkId(theMsgR.mLinkId),
	mDataType(theMsgR.mDataType)
{}

MMsgRoutingBaseDataObject::MMsgRoutingBaseDataObject(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingBaseDataObject& MMsgRoutingBaseDataObject::operator =(const MMsgRoutingBaseDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mLinkId = theMsgR.mLinkId;
		mDataType            = theMsgR.mDataType;
	}

	return *this;
}

void MMsgRoutingBaseDataObject::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Link Id: " << mLinkId << endl;
	os << "  Data Type: " << mDataType << endl;
}