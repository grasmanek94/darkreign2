#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactRunProcess.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactRunProcess;
};

//
// SMsgFactRunProcess
//

SMsgFactRunProcess::SMsgFactRunProcess(void) :
	SmallMessage(), 
	mCommandLine(), 
	mWorkingDirectory(),
	mRunDetached(false)
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactRunProcess);
}

SMsgFactRunProcess::~SMsgFactRunProcess(void)
{}

SMsgFactRunProcess::SMsgFactRunProcess(const SMsgFactRunProcess& theMsgR) :
	SmallMessage(theMsgR),
	mCommandLine(theMsgR.mCommandLine),
	mWorkingDirectory(theMsgR.mWorkingDirectory),
	mRunDetached(theMsgR.mRunDetached)
{}

SMsgFactRunProcess::SMsgFactRunProcess(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactRunProcess& SMsgFactRunProcess::operator=(const SMsgFactRunProcess& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mCommandLine = theMsgR.mCommandLine;
		mWorkingDirectory = theMsgR.mWorkingDirectory;
		mRunDetached = theMsgR.mRunDetached;
	}

	return *this;
}

void SMsgFactRunProcess::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
	os << "  CommandLine: " << mCommandLine << endl;
	os << "  WorkingDirectory: " << mWorkingDirectory << endl;
	os << "  Run detached: " << mRunDetached << endl;
}

void* SMsgFactRunProcess::Pack(void)
{
	WTRACE("SMsgFactRunProcess::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactRunProcess);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactRunProcess::Pack Appending message data");

	Append_PA_STRING(mCommandLine);
	Append_PA_STRING(mWorkingDirectory);
	AppendByte(mRunDetached?1:0);

	return GetDataPtr();
}

void SMsgFactRunProcess::Unpack(void)
{
	WTRACE("SMsgFactRunProcess::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactRunProcess)
	{
		WDBG_AH("SMsgFactRunProcess::Unpack Not a FactRunProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactRunProcess message!");
	}

	WDBG_LL("SMsgFactRunProcess::Unpack Reading message data");

	ReadString(mCommandLine);
	ReadString(mWorkingDirectory);
	mRunDetached = ReadByte()!=0;
}

