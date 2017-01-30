#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStreamFile.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactStreamFile;
};

//
// SMsgFactStreamFile
//

SMsgFactStreamFile::SMsgFactStreamFile(void) :
	SmallMessage(), mCompress(false)
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactStreamFile);

	mModifyTime.dwHighDateTime = 0;
	mModifyTime.dwLowDateTime = 0;
}

SMsgFactStreamFile::~SMsgFactStreamFile(void)
{}

SMsgFactStreamFile::SMsgFactStreamFile(const SMsgFactStreamFile& theMsgR) :
	SmallMessage(theMsgR),
	mFilePath(theMsgR.mFilePath),
	mCompress(theMsgR.mCompress),
	mModifyTime(theMsgR.mModifyTime)
{}

SMsgFactStreamFile::SMsgFactStreamFile(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactStreamFile& SMsgFactStreamFile::operator=(const SMsgFactStreamFile& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mFilePath = theMsgR.mFilePath;
		mCompress = theMsgR.mCompress;
		mModifyTime = theMsgR.mModifyTime;
	}

	return *this;
}

void SMsgFactStreamFile::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  FilePath: " << mFilePath << endl;
	os << "  Compress: " << mCompress << endl;
}

void* SMsgFactStreamFile::Pack(void)
{
	WTRACE("SMsgFactStreamFile::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactStreamFile);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactStreamFile::Pack Appending message data");

	Append_PA_STRING(mFilePath);
	AppendByte(mCompress?1:0);
	AppendLong(mModifyTime.dwHighDateTime);
	AppendLong(mModifyTime.dwLowDateTime);
	return GetDataPtr();
}

void SMsgFactStreamFile::Unpack(void)
{
	WTRACE("SMsgFactStreamFile::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactStreamFile)
	{
		WDBG_AH("SMsgFactStreamFile::Unpack Not a FactStreamFile message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactStreamFile message!");
	}

	WDBG_LL("SMsgFactStreamFile::Unpack Reading message data");

	ReadString(mFilePath);
	mCompress = ReadByte()!=0;
	mModifyTime.dwHighDateTime = ReadLong();
	mModifyTime.dwLowDateTime = ReadLong();
}

