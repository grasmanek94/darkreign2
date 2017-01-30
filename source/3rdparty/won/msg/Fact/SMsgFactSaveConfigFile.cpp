#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactSaveConfigFile.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactSaveConfigFile;
};

//
// SMsgFactSaveConfigFile
//

SMsgFactSaveConfigFile::SMsgFactSaveConfigFile(void) :
	SmallMessage()

{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactSaveConfigFile);
}

SMsgFactSaveConfigFile::~SMsgFactSaveConfigFile(void)
{}

SMsgFactSaveConfigFile::SMsgFactSaveConfigFile(const SMsgFactSaveConfigFile& theMsgR) :
	SmallMessage(theMsgR),
	mConfigName(theMsgR.mConfigName),
	mFilePath(theMsgR.mFilePath),
	mBlob(theMsgR.mBlob)
{}

SMsgFactSaveConfigFile::SMsgFactSaveConfigFile(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactSaveConfigFile& SMsgFactSaveConfigFile::operator=(const SMsgFactSaveConfigFile& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mConfigName = theMsgR.mConfigName;
		mFilePath = theMsgR.mFilePath;
		mBlob = theMsgR.mBlob;
	}

	return *this;
}

void SMsgFactSaveConfigFile::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  ConfigName: " << mConfigName << endl;
	os << "  FilePath: " << mFilePath << endl;
	os << "  Blob: " << mBlob << endl;
}

void* SMsgFactSaveConfigFile::Pack(void)
{
	WTRACE("SMsgFactSaveConfigFile::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactSaveConfigFile);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactSaveConfigFile::Pack Appending message data");

	Append_PA_STRING(mConfigName);	Append_PA_STRING(mFilePath);	AppendRawString(mBlob);
	return GetDataPtr();
}

void SMsgFactSaveConfigFile::Unpack(void)
{
	WTRACE("SMsgFactSaveConfigFile::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactSaveConfigFile)
	{
		WDBG_AH("SMsgFactSaveConfigFile::Unpack Not a FactSaveConfigFile message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactSaveConfigFile message!");
	}

	WDBG_LL("SMsgFactSaveConfigFile::Unpack Reading message data");

	ReadString(mConfigName);
	ReadString(mFilePath);
	ReadRawString(mBlob);
}

