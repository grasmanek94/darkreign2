

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesFact.h"
#include "TMsgFactStartProcessUnicode.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgFactStartProcessBase;
	using WONMsg::TMsgFactStartProcessUnicode;
};


TMsgFactStartProcessUnicode::TMsgFactStartProcessUnicode(void) : TMsgFactStartProcessBase()
{
	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStartProcessUnicode);
}


TMsgFactStartProcessUnicode::~TMsgFactStartProcessUnicode(void){
}


TMsgFactStartProcessUnicode::TMsgFactStartProcessUnicode(const TMsgFactStartProcessUnicode& theMsgR) :
    TMsgFactStartProcessBase(theMsgR)
{}


TMsgFactStartProcessUnicode::TMsgFactStartProcessUnicode(const TMessage& theMsgR) : 
    TMsgFactStartProcessBase(theMsgR)
{
	Unpack();
}


TMsgFactStartProcessUnicode& TMsgFactStartProcessUnicode::operator =(const TMsgFactStartProcessUnicode& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMsgFactStartProcessBase::operator=(theMsgR);
	return *this;

}


void TMsgFactStartProcessUnicode::PackCommandLine(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Pack");

	Append_PW_STRING(mCmdLine);
}


void TMsgFactStartProcessUnicode::UnpackCommandLine(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Unpack");

	ReadWString(mCmdLine);
}

void* TMsgFactStartProcessUnicode::Pack(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Pack");

	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStartProcessUnicode);
	return TMsgFactStartProcessBase::Pack();
}

void TMsgFactStartProcessUnicode::Unpack(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Unpack");

	TMsgFactStartProcessBase::Unpack();

	if ((GetServiceType() != WONMsg::FactoryServer) ||
	    (GetMessageType() != WONMsg::FactStartProcessUnicode))
	{
		WDBG_AH("TMsgFactStartProcessUnicode::Unpack Not a FactStartProcessUnicode message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcessUnicode message.");
	}
}
