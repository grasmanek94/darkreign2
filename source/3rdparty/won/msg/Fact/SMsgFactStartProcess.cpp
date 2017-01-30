

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStartProcess.h"

using namespace WONMsg;

SMsgFactStartProcess::SMsgFactStartProcess(void) : SMsgFactStartProcessBase()
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStartProcess);
}


SMsgFactStartProcess::~SMsgFactStartProcess(void){
}


SMsgFactStartProcess::SMsgFactStartProcess(const SMsgFactStartProcess& theMsgR) :
    SMsgFactStartProcessBase(theMsgR)
{}


SMsgFactStartProcess::SMsgFactStartProcess(const SmallMessage& theMsgR) : 
    SMsgFactStartProcessBase(theMsgR) 
{
	Unpack();
}


SMsgFactStartProcess& SMsgFactStartProcess::operator =(const SMsgFactStartProcess& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SMsgFactStartProcessBase::operator=(theMsgR);

	return *this;

}


void SMsgFactStartProcess::PackCommandLine(void)
{
	WTRACE("SMsgFactStartProcess::PackCommandLine");

	Append_PA_STRING(WONCommon::WStringToString(mCmdLine));
}


void SMsgFactStartProcess::UnpackCommandLine(void)
{
	WTRACE("SMsgFactStartProcess::Unpack");

	std::string aString;
	ReadString(aString);
	mCmdLine = WONCommon::StringToWString(aString);
}

void* SMsgFactStartProcess::Pack(void)
{
	WTRACE("SMsgFactStartProcess::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStartProcess);
	return SMsgFactStartProcessBase::Pack();
}

void SMsgFactStartProcess::Unpack(void)
{
	WTRACE("SMsgFactStartProcess::Unpack");

	SMsgFactStartProcessBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::SmallFactStartProcess))
	{
		WDBG_AH("SMsgFactStartProcessBase::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}
}
