#include "RoutingServerDirEntity.h"

const unsigned char* OBJ_CLIENTCOUNT = (unsigned char*)"__RSClientCount";
const unsigned char* OBJ_ROOMFLAGS   = (unsigned char*)"__RSRoomFlags";
const unsigned char* OBJ_LOGINACL    = (unsigned char*)"__RSLoginACL";

RoutingServerDirEntity::RoutingServerDirEntity() :
	mIsPasswordProtected(false),
	mIsFiltered(false),
	mClientCount(0)
{}

void RoutingServerDirEntity::Parse()
{
	// if it's not a Routing Server, abort
	if (mType != ET_SERVICE || mName != L"TitanRoutingServer")
		return;

	WONCommon::DataObjectTypeSet::const_iterator aObjItr = mDataObjects.begin();
	for( ; aObjItr != mDataObjects.end(); ++aObjItr)
	{
		if (aObjItr->GetDataType() == OBJ_CLIENTCOUNT)
			mClientCount = *(unsigned long*)aObjItr->GetData().c_str();
		else if (aObjItr->GetDataType() == OBJ_LOGINACL)
			;
		else if (aObjItr->GetDataType() == OBJ_ROOMFLAGS)
		{
			mIsPasswordProtected = ((*aObjItr->GetData().c_str()) & 0x1 != 0);
			mIsFiltered          = ((*aObjItr->GetData().c_str()) & 0x2 != 0);
		}
	}
}
