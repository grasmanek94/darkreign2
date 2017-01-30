#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactPullConfigFile.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactPullConfigFile;
};

//
// SMsgFactPullConfigFile
//

SMsgFactPullConfigFile::SMsgFactPullConfigFile(void) :
	SmallMessage()

{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactPullConfigFile);
}

SMsgFactPullConfigFile::~SMsgFactPullConfigFile(void)
{}

SMsgFactPullConfigFile::SMsgFactPullConfigFile(const SMsgFactPullConfigFile& theMsgR) :
	SmallMessage(theMsgR),
	mConfigName(theMsgR.mConfigName),
	mFilePath(theMsgR.mFilePath)
{}

SMsgFactPullConfigFile::SMsgFactPullConfigFile(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactPullConfigFile& SMsgFactPullConfigFile::operator=(const SMsgFactPullConfigFile& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mConfigName = theMsgR.mConfigName;
		mFilePath = theMsgR.mFilePath;
	}

	return *this;
}

void SMsgFactPullConfigFile::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  ConfigName: " << mConfigName << endl;
	os << "  FilePath: " << mFilePath << endl;
}

void* SMsgFactPullConfigFile::Pack(void)
{
	WTRACE("SMsgFactPullConfigFile::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactPullConfigFile);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactPullConfigFile::Pack Appending message data");

	Append_PA_STRING(mConfigName);	Append_PA_STRING(mFilePath);	
	return GetDataPtr();
}

void SMsgFactPullConfigFile::Unpack(void)
{
	WTRACE("SMsgFactPullConfigFile::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactPullConfigFile)
	{
		WDBG_AH("SMsgFactPullConfigFile::Unpack Not a FactPullConfigFile message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactPullConfigFile message!");
	}

	WDBG_LL("SMsgFactPullConfigFile::Unpack Reading message data");

	ReadString(mConfigName);
	ReadString(mFilePath);
}

