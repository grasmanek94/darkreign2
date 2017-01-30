

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetFreeDiskSpaceR.h"

using namespace WONMsg;



SMsgFactGetFreeDiskSpaceReply::SMsgFactGetFreeDiskSpaceReply(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFreeDiskSpaceReply);

}


SMsgFactGetFreeDiskSpaceReply::~SMsgFactGetFreeDiskSpaceReply(void){
}


SMsgFactGetFreeDiskSpaceReply::SMsgFactGetFreeDiskSpaceReply(const SMsgFactGetFreeDiskSpaceReply& theMsgR) : 
	SmallMessage(theMsgR), mFreeDiskSpaceList(theMsgR.mFreeDiskSpaceList)
{
}


SMsgFactGetFreeDiskSpaceReply::SMsgFactGetFreeDiskSpaceReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR)
{
	Unpack();
}


SMsgFactGetFreeDiskSpaceReply& SMsgFactGetFreeDiskSpaceReply::operator =(const SMsgFactGetFreeDiskSpaceReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mFreeDiskSpaceList = theMsgR.mFreeDiskSpaceList;
	return *this;

}


void* SMsgFactGetFreeDiskSpaceReply::Pack(void){

	WTRACE("SMsgFactGetFreeDiskSpaceReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFreeDiskSpaceReply);
	SmallMessage::Pack();

	AppendShort(mFreeDiskSpaceList.size());

	FreeDiskSpaceList::iterator anItr = mFreeDiskSpaceList.begin();
	while(anItr!=mFreeDiskSpaceList.end())
	{
		Append_PA_STRING(anItr->first);
		AppendBytes(sizeof(__int64),&anItr->second.mTotal);
		AppendBytes(sizeof(__int64),&anItr->second.mFree);

		anItr++;
	}

	return GetDataPtr();

}


void SMsgFactGetFreeDiskSpaceReply::Unpack(void){
 
	WTRACE("SMsgFactGetFreeDiskSpaceReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetFreeDiskSpaceReply))
	{
		WDBG_AH("SMsgFactGetFreeDiskSpaceReply::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactGetFreeDiskSpaceReply::Unpack Reading message data");

	mFreeDiskSpaceList.clear();

	unsigned short aNumDisks = ReadShort();
	for(int n=0; n<aNumDisks; n++)
	{
		FreeDiskSpacePair aPair;
		ReadString(aPair.first);
		aPair.second.mTotal = *(__int64*)ReadBytes(sizeof(__int64));
		aPair.second.mFree = *(__int64*)ReadBytes(sizeof(__int64));

		mFreeDiskSpaceList.push_back(aPair);
	}

}
