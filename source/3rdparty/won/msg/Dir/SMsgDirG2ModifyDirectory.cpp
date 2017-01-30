// SMsgDirG2ModifyDirectory

// DirectoryServer modify directory message.  Modifies attributes of a directory,
// inclduing its name (key field).


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2ModifyDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateExtendBase;
	using WONMsg::SMsgDirG2ModifyDirectory;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2ModifyDirectory::SMsgDirG2ModifyDirectory(bool isExtended) :
	SMsgDirG2UpdateExtendBase(KT_DIRECTORY, isExtended, isExtended),
	mEntityFlags(0),
	mNewName(),
	mNewDisplayName(),
	mNewLifespan(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(IsExtended() ? WONMsg::DirG2ModifyDirectoryEx : WONMsg::DirG2ModifyDirectory);
}


// SmallMessage ctor
SMsgDirG2ModifyDirectory::SMsgDirG2ModifyDirectory(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(0),
	mNewName(),
	mNewDisplayName(),
	mNewLifespan(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2ModifyDirectory::SMsgDirG2ModifyDirectory(const SMsgDirG2ModifyDirectory& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(theMsgR.mEntityFlags),
	mNewName(theMsgR.mNewName),
	mNewDisplayName(theMsgR.mNewDisplayName),
	mNewLifespan(theMsgR.mNewLifespan)
{}


// Destructor
SMsgDirG2ModifyDirectory::~SMsgDirG2ModifyDirectory(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2ModifyDirectory&
SMsgDirG2ModifyDirectory::operator=(const SMsgDirG2ModifyDirectory& theMsgR)
{
	SMsgDirG2UpdateExtendBase::operator=(theMsgR);
	mEntityFlags    = theMsgR.mEntityFlags;
	mNewName        = theMsgR.mNewName;
	mNewDisplayName = theMsgR.mNewDisplayName;
	mNewLifespan    = theMsgR.mNewLifespan;
	return *this;
}


// SMsgDirG2ModifyDirectory::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2ModifyDirectory::Pack(void)
{
	WTRACE("SMsgDirG2ModifyDirectory::Pack");
	SetKeyType(KT_DIRECTORY);
	SetServiceType(WONMsg::SmallDirServerG2);
	if (IsExtended())
	{
		SetMessageType(WONMsg::DirG2ModifyDirectoryEx);
		SetExtended(true, true);
	}
	else
		SetMessageType(WONMsg::DirG2ModifyDirectory);

	SMsgDirG2UpdateExtendBase::Pack();
	AppendByte(mEntityFlags);
	PackKey(*this);
	Append_PW_STRING(mNewName);
	Append_PW_STRING(mNewDisplayName);
	AppendLong(mNewLifespan);
	PackExtended();
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2ModifyDirectory::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2ModifyDirectory::Unpack(void)
{
	WTRACE("SMsgDirG2ModifyDirectory::Unpack");
	SetKeyType(KT_DIRECTORY);
	SetExtended((GetMessageType() != WONMsg::DirG2ModifyDirectory), (GetMessageType() == WONMsg::DirG2ModifyDirectoryEx));
	SMsgDirG2UpdateExtendBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2ModifyDirectory) &&
	     (GetMessageType() != WONMsg::DirG2ModifyDirectoryEx) &&
		 (GetMessageType() != WONMsg::DirG2ModifyDirectoryExObsolete)))
	{
		WDBG_AH("SMsgDirG2ModifyDirectory::Unpack Not a DirG2ModifyDirectory(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2ModifyDirectory(Ex) message.");
	}

	mEntityFlags = ReadByte();
	UnpackKey(*this);
	ReadWString(mNewName);
	ReadWString(mNewDisplayName);
	mNewLifespan = ReadLong();
	UnpackExtended();
	UnpackPeerData();
}
