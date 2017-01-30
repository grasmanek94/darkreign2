

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStartProcessBase.h"

using namespace WONMsg;

SMsgFactStartProcessBase::SMsgFactStartProcessBase(void) : SmallMessage(), AllMsgStartProcessBase()
{}


SMsgFactStartProcessBase::~SMsgFactStartProcessBase(void){
}


SMsgFactStartProcessBase::SMsgFactStartProcessBase(const SMsgFactStartProcessBase& theMsgR) :
    SmallMessage(theMsgR), AllMsgStartProcessBase(theMsgR)
{
}


SMsgFactStartProcessBase::SMsgFactStartProcessBase(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR), AllMsgStartProcessBase()
{}


SMsgFactStartProcessBase& SMsgFactStartProcessBase::operator =(const SMsgFactStartProcessBase& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	AllMsgStartProcessBase::operator=(theMsgR);

	return *this;
}


void* SMsgFactStartProcessBase::Pack(void){

	WTRACE("SMsgFactStartProcessBase::Pack");

	SmallMessage::Pack();

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


void SMsgFactStartProcessBase::Unpack(void){

	WTRACE("SMsgFactStartProcessBase::Unpack");

	SmallMessage::Unpack();

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
