

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesFact.h"
#include "TMsgFactStartProcessBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgFactStartProcessBase;
};


TMsgFactStartProcessBase::TMsgFactStartProcessBase(void) : TMessage(), AllMsgStartProcessBase()
{}


TMsgFactStartProcessBase::~TMsgFactStartProcessBase(void){
}


TMsgFactStartProcessBase::TMsgFactStartProcessBase(const TMsgFactStartProcessBase& theMsgR) :
    TMessage(theMsgR), AllMsgStartProcessBase()
{
}


TMsgFactStartProcessBase::TMsgFactStartProcessBase(const TMessage& theMsgR) : 
    TMessage(theMsgR), AllMsgStartProcessBase() 
{}


TMsgFactStartProcessBase& TMsgFactStartProcessBase::operator =(const TMsgFactStartProcessBase& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMessage::operator=(theMsgR);
	AllMsgStartProcessBase::operator=(theMsgR);

	return *this;

}


void* TMsgFactStartProcessBase::Pack(void){

	WTRACE("TMsgFactStartProcessBase::Pack");

	TMessage::Pack();

	WDBG_LL("TMsgFactStartProcessBase::Pack Appending message data");
	Append_PA_STRING(mProcessName);
	AppendByte(mAddCmdLine);
	PackCommandLine();
	AppendLong(static_cast<unsigned long>(mWaitTime));
	
	Append_PA_STRING(mDirServerAdress);
	Append_PW_STRING(mDisplayName);
	Append_PW_STRING(mRegisterDir);
	AppendByte(mAbortRegFail);

	AppendByte(mTotalPorts);
	AppendByte(mPortSet.size());
	for(FACT_SERV_PORT_RANGE_SET::iterator aPortIterator(mPortSet.begin());
		aPortIterator != mPortSet.end(); aPortIterator++){

		AppendShort(*aPortIterator);

	}

	AppendShort(mAuthorizedIPSet.size());
	for(FACT_SERV_TRUSTED_ADDRESS_SET::iterator anIterator(mAuthorizedIPSet.begin());
		anIterator != mAuthorizedIPSet.end(); anIterator++){

		Append_PA_STRING(*anIterator);

	}

	return GetDataPtr();
}


void TMsgFactStartProcessBase::Unpack(void){

	WTRACE("TMsgFactStartProcessBase::Unpack");

	TMessage::Unpack();

	WDBG_LL("TMsgFactStartProcessBase::Unpack Reading message data");
	ReadString(mProcessName);
	mAddCmdLine = (ReadByte()) ? true : false;
	UnpackCommandLine();
	mWaitTime = static_cast<long>(ReadLong());

	ReadString(mDirServerAdress);
	mEmptyDirServerAddress = mDirServerAdress.empty();

	ReadWString(mDisplayName);
	ReadWString(mRegisterDir);
	mAbortRegFail = (ReadByte()) ? true : false;
	mTotalPorts = ReadByte();

	unsigned char aNumberPorts(ReadByte());
	for(unsigned char i(0); i < aNumberPorts; i++){
		mPortSet.insert(ReadShort());
	}

	unsigned short aNumberIP(ReadShort());
	int aPos;
	for(unsigned short j(0); j < aNumberIP; j++){

		string anAddressStr;
		ReadString(anAddressStr);
		// Remove the port portion of the address
		aPos = anAddressStr.find(':');
		if(aPos != std::string::npos){
			anAddressStr = anAddressStr.substr(0, aPos);
		}
		mAuthorizedIPSet.insert(anAddressStr);
	}
}
