#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetDir.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactGetDir;
};

//
// SMsgFactGetDir
//

SMsgFactGetDir::SMsgFactGetDir(void) :
	SmallMessage()
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetDir);
}

SMsgFactGetDir::~SMsgFactGetDir(void)
{}

SMsgFactGetDir::SMsgFactGetDir(const SMsgFactGetDir& theMsgR) :
	SmallMessage(theMsgR),
	mFilePath(theMsgR.mFilePath)
{}

SMsgFactGetDir::SMsgFactGetDir(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactGetDir& SMsgFactGetDir::operator=(const SMsgFactGetDir& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mFilePath = theMsgR.mFilePath;
	}

	return *this;
}

void SMsgFactGetDir::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  FilePath: " << mFilePath << endl;
}

void* SMsgFactGetDir::Pack(void)
{
	WTRACE("SMsgFactGetDir::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetDir);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetDir::Pack Appending message data");

	Append_PA_STRING(mFilePath);
	return GetDataPtr();
}

void SMsgFactGetDir::Unpack(void)
{
	WTRACE("SMsgFactGetDir::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactGetDir)
	{
		WDBG_AH("SMsgFactGetDir::Unpack Not a FactGetDir message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactGetDir message!");
	}

	WDBG_LL("SMsgFactGetDir::Unpack Reading message data");

	ReadString(mFilePath);
}

