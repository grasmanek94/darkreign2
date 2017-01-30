

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStartProcessUnicode.h"

using namespace WONMsg;

SMsgFactStartProcessUnicode::SMsgFactStartProcessUnicode(void) : SMsgFactStartProcessBase()
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStartProcessUnicode);
}


SMsgFactStartProcessUnicode::~SMsgFactStartProcessUnicode(void){
}


SMsgFactStartProcessUnicode::SMsgFactStartProcessUnicode(const SMsgFactStartProcessUnicode& theMsgR) :
    SMsgFactStartProcessBase(theMsgR)
{}


SMsgFactStartProcessUnicode::SMsgFactStartProcessUnicode(const SmallMessage& theMsgR) : 
    SMsgFactStartProcessBase(theMsgR)
{
	Unpack();
}


SMsgFactStartProcessUnicode& SMsgFactStartProcessUnicode::operator =(const SMsgFactStartProcessUnicode& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SMsgFactStartProcessBase::operator=(theMsgR);

	return *this;

}


void SMsgFactStartProcessUnicode::PackCommandLine(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Pack");

	Append_PW_STRING(mCmdLine);
}


void SMsgFactStartProcessUnicode::UnpackCommandLine(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Unpack");

	ReadWString(mCmdLine);
}

void* SMsgFactStartProcessUnicode::Pack(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStartProcessUnicode);
	return SMsgFactStartProcessBase::Pack();
}

void SMsgFactStartProcessUnicode::Unpack(void)
{
	WTRACE("TMsgFactStartProcessUnicode::Unpack");

	SMsgFactStartProcessBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::SmallFactStartProcessUnicode))
	{
		WDBG_AH("SMsgFactStartProcessUnicode::Unpack Not a FactStartProcessUnicode message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcessUnicode message.");
	}
}
