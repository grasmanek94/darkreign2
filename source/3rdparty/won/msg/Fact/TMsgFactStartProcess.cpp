

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesFact.h"
#include "TMsgFactStartProcess.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgFactStartProcessBase;
	using WONMsg::TMsgFactStartProcess;
};


TMsgFactStartProcess::TMsgFactStartProcess(void) : TMsgFactStartProcessBase()
{
	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStartProcess);
}


TMsgFactStartProcess::~TMsgFactStartProcess(void){
}


TMsgFactStartProcess::TMsgFactStartProcess(const TMsgFactStartProcess& theMsgR) :
    TMsgFactStartProcessBase(theMsgR)
{}


TMsgFactStartProcess::TMsgFactStartProcess(const TMessage& theMsgR) : 
    TMsgFactStartProcessBase(theMsgR) 
{
	Unpack();
}


TMsgFactStartProcess& TMsgFactStartProcess::operator =(const TMsgFactStartProcess& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMsgFactStartProcessBase::operator=(theMsgR);

	return *this;

}


void TMsgFactStartProcess::PackCommandLine(void)
{
	WTRACE("TMsgFactStartProcess::PackCommandLine");

	Append_PA_STRING(WONCommon::WStringToString(mCmdLine));
}


void TMsgFactStartProcess::UnpackCommandLine(void)
{
	WTRACE("TMsgFactStartProcess::Unpack");

	std::string aString;
	ReadString(aString);
	mCmdLine = WONCommon::StringToWString(aString);
}

void* TMsgFactStartProcess::Pack(void)
{
	WTRACE("TMsgFactStartProcess::Pack");

	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStartProcess);
	return TMsgFactStartProcessBase::Pack();
}

void TMsgFactStartProcess::Unpack(void)
{
	WTRACE("TMsgFactStartProcess::Unpack");

	TMsgFactStartProcessBase::Unpack();

	if ((GetServiceType() != WONMsg::FactoryServer) ||
	    (GetMessageType() != WONMsg::FactStartProcess))
	{
		WDBG_AH("TMsgFactStartProcessBase::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}
}
