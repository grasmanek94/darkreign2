// SMsgDirG2AddDirectory

// DirectoryServer add directory message.  Adds a new directory at specified path.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2AddDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateExtendBase;
	using WONMsg::SMsgDirG2AddDirectory;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2AddDirectory::SMsgDirG2AddDirectory(bool isExtended) :
	SMsgDirG2UpdateExtendBase(KT_DIRECTORY, isExtended, isExtended),
	mEntityFlags(0),
	mDirName(),
	mDisplayName(),
	mLifespan(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(IsExtended() ? WONMsg::DirG2AddDirectoryEx : WONMsg::DirG2AddDirectory);
}


// SmallMessage ctor
SMsgDirG2AddDirectory::SMsgDirG2AddDirectory(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(0),
	mDirName(),
	mDisplayName(),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2AddDirectory::SMsgDirG2AddDirectory(const SMsgDirG2AddDirectory& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(theMsgR.mEntityFlags),
	mDirName(theMsgR.mDirName),
	mDisplayName(theMsgR.mDisplayName),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
SMsgDirG2AddDirectory::~SMsgDirG2AddDirectory(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2AddDirectory&
SMsgDirG2AddDirectory::operator=(const SMsgDirG2AddDirectory& theMsgR)
{
	SMsgDirG2UpdateExtendBase::operator=(theMsgR);
	mEntityFlags = theMsgR.mEntityFlags;
	mDirName     = theMsgR.mDirName;
	mDisplayName = theMsgR.mDisplayName;
	mLifespan    = theMsgR.mLifespan;
	return *this;
}


// SMsgDirG2AddDirectory::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2AddDirectory::Pack(void)
{
	WTRACE("SMsgDirG2AddDirectory::Pack");
	SetKeyType(KT_DIRECTORY);
	SetServiceType(WONMsg::SmallDirServerG2);
	if (IsExtended())
	{
		SetMessageType(WONMsg::DirG2AddDirectoryEx);
		SetExtended(true, true);
	}
	else
		SetMessageType(WONMsg::DirG2AddDirectory);

	SMsgDirG2UpdateExtendBase::Pack();
	AppendByte(mEntityFlags);
	PackKey(*this);
	Append_PW_STRING(mDirName);
	Append_PW_STRING(mDisplayName);
	AppendLong(mLifespan);
	PackExtended();
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2AddDirectory::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2AddDirectory::Unpack(void)
{
	WTRACE("SMsgDirG2AddDirectory::Unpack");
	SetKeyType(KT_DIRECTORY);
	SetExtended((GetMessageType() != WONMsg::DirG2AddDirectory), (GetMessageType() == WONMsg::DirG2AddDirectoryEx));
	SMsgDirG2UpdateExtendBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2AddDirectory) &&
	     (GetMessageType() != WONMsg::DirG2AddDirectoryEx) &&
		 (GetMessageType() != WONMsg::DirG2AddDirectoryExObsolete)))
	{
		WDBG_AH("SMsgDirG2AddDirectory::Unpack Not a DirG2AddDirectory(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2AddDirectory(Ex) message.");
	}

	mEntityFlags = ReadByte();
	UnpackKey(*this);
	ReadWString(mDirName);
	ReadWString(mDisplayName);
	mLifespan = ReadLong();
	UnpackExtended();
	UnpackPeerData();
}
