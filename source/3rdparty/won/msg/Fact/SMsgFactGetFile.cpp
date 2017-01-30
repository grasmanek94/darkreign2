#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetFile.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactGetFile;
};

//
// SMsgFactGetFile
//

SMsgFactGetFile::SMsgFactGetFile(void) :
	SmallMessage(), mCompress(false)
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFile);

	mBlockLen = 100000;
}

SMsgFactGetFile::~SMsgFactGetFile(void)
{}

SMsgFactGetFile::SMsgFactGetFile(const SMsgFactGetFile& theMsgR) :
	SmallMessage(theMsgR),
	mFilePath(theMsgR.mFilePath),
	mCompress(theMsgR.mCompress),
	mBlockLen(theMsgR.mBlockLen)
{}

SMsgFactGetFile::SMsgFactGetFile(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactGetFile& SMsgFactGetFile::operator=(const SMsgFactGetFile& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mFilePath = theMsgR.mFilePath;
		mCompress = theMsgR.mCompress;
		mBlockLen = theMsgR.mBlockLen;
	}

	return *this;
}

void SMsgFactGetFile::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  FilePath: " << mFilePath << endl;
	os << "  Compress: " << mCompress << endl;
}

void* SMsgFactGetFile::Pack(void)
{
	WTRACE("SMsgFactGetFile::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFile);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetFile::Pack Appending message data");

	Append_PA_STRING(mFilePath);
	AppendByte(mCompress?1:0);
	AppendLong(mBlockLen);
	return GetDataPtr();
}

void SMsgFactGetFile::Unpack(void)
{
	WTRACE("SMsgFactGetFile::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactGetFile)
	{
		WDBG_AH("SMsgFactGetFile::Unpack Not a FactGetFile message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactGetFile message!");
	}

	WDBG_LL("SMsgFactGetFile::Unpack Reading message data");

	ReadString(mFilePath);
	mCompress = ReadByte()!=0;
	mBlockLen = ReadLong();
}

