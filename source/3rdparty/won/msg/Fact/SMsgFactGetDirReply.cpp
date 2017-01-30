#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetDirReply.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactGetDirReply;
};

//
// SMsgFactGetDirReply
//

SMsgFactGetDirReply::SMsgFactGetDirReply(void) :
	SmallMessage(), mStatus(0)
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetDirReply);
}

SMsgFactGetDirReply::~SMsgFactGetDirReply(void)
{}

SMsgFactGetDirReply::SMsgFactGetDirReply(const SMsgFactGetDirReply& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus), mFileList(theMsgR.mFileList)
{}

SMsgFactGetDirReply::SMsgFactGetDirReply(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactGetDirReply& SMsgFactGetDirReply::operator=(const SMsgFactGetDirReply& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mStatus = theMsgR.mStatus;
		mFileList = theMsgR.mFileList;
	}

	return *this;
}

void SMsgFactGetDirReply::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
}

void* SMsgFactGetDirReply::Pack(void)
{
	WTRACE("SMsgFactGetDirReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetDirReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetDirReply::Pack Appending message data");

	AppendShort(mStatus);
	AppendShort(mFileList.size());
	FileList::iterator anItr = mFileList.begin();
	while(anItr!=mFileList.end())
	{
		Append_PA_STRING(anItr->mName);
		AppendLong(anItr->mAttributes);
		AppendBytes(sizeof(FILETIME),&(anItr->mModifyTime));
		AppendBytes(sizeof(__int64),&(anItr->mFileSize));
		++anItr;
	}

	return GetDataPtr();
}

void SMsgFactGetDirReply::Unpack(void)
{
	WTRACE("SMsgFactGetDirReply::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactGetDirReply)
	{
		WDBG_AH("SMsgFactGetDirReply::Unpack Not a FactGetDirReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactGetDirReply message!");
	}

	WDBG_LL("SMsgFactGetDirReply::Unpack Reading message data");

	mStatus = ReadShort();
	int aNumEntries = ReadShort();

	string aName;
	DWORD anAttributes;
	FILETIME aModifyTime;
	__int64 aFileSize;

	mFileList.clear();

	for(int i=0; i<aNumEntries; i++)
	{
		ReadString(aName);
		anAttributes = ReadLong();
		memcpy(&aModifyTime, ReadBytes(sizeof(FILETIME)), sizeof(FILETIME));
		memcpy(&aFileSize, ReadBytes(sizeof(__int64)), sizeof(__int64));

		mFileList.push_back(FileEntry(aName, anAttributes, aModifyTime, aFileSize));
	}
}

