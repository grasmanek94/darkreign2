#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesComm.h"
#include "SMsgCommRegisterRequest.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgCommRegisterRequest;
};


SMsgCommRegisterRequest::SMsgCommRegisterRequest(bool isExtended) :
	SmallMessage(),
	mIsExtended(isExtended),
	mDisplayName(),
	mPath()
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::SmallCommRegisterRequest);
}


SMsgCommRegisterRequest::~SMsgCommRegisterRequest(void)
{}


SMsgCommRegisterRequest::SMsgCommRegisterRequest(const SMsgCommRegisterRequest& theMsgR) :
    SmallMessage(theMsgR),
	mIsExtended(theMsgR.mIsExtended),
	mDirServerAddressList(theMsgR.mDirServerAddressList),
	mDisplayName(theMsgR.mDisplayName),
	mPath(theMsgR.mPath),
	mDataObjects(theMsgR.mDataObjects)
{}


SMsgCommRegisterRequest::SMsgCommRegisterRequest(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR)
{
	Unpack();
}


SMsgCommRegisterRequest& SMsgCommRegisterRequest::operator =(const SMsgCommRegisterRequest& theMsgR)
{
	if(this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mDirServerAddressList = theMsgR.mDirServerAddressList;
		mIsExtended           = theMsgR.mIsExtended;
		mDisplayName          = theMsgR.mDisplayName;
		mPath                 = theMsgR.mPath;
		mDataObjects          = theMsgR.mDataObjects;
	}

	return *this;
}


void* SMsgCommRegisterRequest::Pack(void)
{
	WTRACE("SMsgCommRegisterRequest::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(mIsExtended ? WONMsg::SmallCommRegisterRequestEx : WONMsg::SmallCommRegisterRequest);
	SmallMessage::Pack();

	WDBG_LL("SMsgCommRegisterRequest::Pack Appending message data");
	AppendByte(mRequireUniqueDisplayName);
	
	// Number of DirServer addresses (byte) followed by the addresses themselves
	AppendByte(mDirServerAddressList.size());
	std::list<std::string>::const_iterator aAddrItr = mDirServerAddressList.begin();
	for ( ; aAddrItr != mDirServerAddressList.end(); ++aAddrItr)
		Append_PA_STRING(*aAddrItr);
	
	Append_PW_STRING(mDisplayName);
	Append_PW_STRING(mPath);
	PackExtended();

	return GetDataPtr();
}


void SMsgCommRegisterRequest::Unpack(void)
{
	WTRACE("SMsgCommRegisterRequest::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallCommonService ||
	    (GetMessageType() != WONMsg::SmallCommRegisterRequest &&
		 GetMessageType() != WONMsg::SmallCommRegisterRequestEx))
	{
		WDBG_AH("SMsgCommRegisterRequest::Unpack Not a SMsgCommRegisterRequest(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a SMsgCommRegisterRequest(Ex) message.");
	}

	WDBG_LL("SMsgCommRegisterRequest::Unpack Reading message data");
	mIsExtended = (GetMessageType() == WONMsg::SmallCommRegisterRequestEx);
	mRequireUniqueDisplayName = (ReadByte() != 0);
	
	// Read number of addresses followed by the address strings themselves
	unsigned char aNumDirServerAddresses = ReadByte();
	for (int iAddr = 0; iAddr < aNumDirServerAddresses; ++iAddr)
	{
		string aAddrString;
		ReadString(aAddrString);
		mDirServerAddressList.push_back(aAddrString);
	}
	
	ReadWString(mDisplayName);
	ReadWString(mPath);
	UnpackExtended();
}

void SMsgCommRegisterRequest::PackExtended(void)
{
	WTRACE("SMsgCommRegisterRequest::PackExtended");
	WDBG_LL("SMsgCommRegisterRequest::PackExtended Extended=" << mIsExtended << " DataObjects size=" << mDataObjects.size());
	if (!mIsExtended) return;

	AppendShort(mDataObjects.size());
	WONCommon::DataObjectTypeSet::iterator anItr(mDataObjects.begin());
	for (; anItr != mDataObjects.end(); anItr++)
	{
		unsigned char aTypeLen = anItr->GetDataType().size();
		AppendByte(aTypeLen);
		if (aTypeLen > 0)
			AppendBytes(aTypeLen, anItr->GetDataType().data());

		unsigned short aDataLen = anItr->GetData().size();
		AppendShort(aDataLen);
		if (aDataLen > 0)
			AppendBytes(aDataLen, anItr->GetData().data());
	}
}


void SMsgCommRegisterRequest::UnpackExtended(void)
{
	WTRACE("SMsgCommRegisterRequest::UnpackExtended");
	WDBG_LL("SMsgCommRegisterRequest::UnpackExtended Extended=" << mIsExtended);
	mDataObjects.clear();
	if (!mIsExtended) return;

	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgCommRegisterRequest::UnpackExtended Reading " << aCt << " entries.");

	for (int i=0; i < aCt; i++)
	{
		WONCommon::DataObject anObj;
		unsigned char aTypeLen = ReadByte();
		if (aTypeLen > 0)
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aTypeLen)), aTypeLen);

		unsigned short aDataLen = ReadShort();
		if (aDataLen > 0)
			anObj.GetData().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataLen)), aDataLen);

		mDataObjects.insert(anObj);
	}
}