// SMsgDirG2FindDirectory

// DirectoryServer find directory message.  Locates one or more directories based on
// key fields.  Fields of directories(s) and their contents included in reply are
// configurable.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2QueryFindBase.h"
#include "SMsgDirG2FindDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryFindBase;
	using WONMsg::SMsgDirG2FindDirectory;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2FindDirectory::SMsgDirG2FindDirectory(bool isExtended) :
	SMsgDirG2QueryFindBase(KT_DIRECTORY, isExtended),
	mDirName()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mExtended ? WONMsg::DirG2FindDirectoryEx : WONMsg::DirG2FindDirectory);
}


// SmallMessage ctor
SMsgDirG2FindDirectory::SMsgDirG2FindDirectory(const SmallMessage& theMsgR) :
	SMsgDirG2QueryFindBase(theMsgR),
	mDirName()
{
	Unpack();
}


// Copy ctor
SMsgDirG2FindDirectory::SMsgDirG2FindDirectory(const SMsgDirG2FindDirectory& theMsgR) :
	SMsgDirG2QueryFindBase(theMsgR),
	mDirName(theMsgR.mDirName)
{}


// Destructor
SMsgDirG2FindDirectory::~SMsgDirG2FindDirectory(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2FindDirectory&
SMsgDirG2FindDirectory::operator=(const SMsgDirG2FindDirectory& theMsgR)
{
	SMsgDirG2QueryFindBase::operator=(theMsgR);
	mDirName = theMsgR.mDirName;
	return *this;
}


// SMsgDirG2FindDirectory::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2FindDirectory::Pack(void)
{
	WTRACE("SMsgDirG2FindDirectory::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mExtended ? WONMsg::DirG2FindDirectoryEx : WONMsg::DirG2FindDirectory);

	SMsgDirG2QueryFindBase::Pack();
	AppendShort(mEntitiesPerReply);

	PackKey(*this);
	Append_PW_STRING(mDirName);
	Append_PW_STRING(mDisplayName);
	PackSearchObjects();
	PackExtended();
	return GetDataPtr();
}


// SMsgDirG2FindDirectory::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2FindDirectory::Unpack(void)
{
	WTRACE("SMsgDirG2FindDirectory::Unpack");
	mExtended = (GetMessageType() == WONMsg::DirG2FindDirectoryEx);
	SMsgDirG2QueryFindBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2FindDirectory) &&
	     (GetMessageType() != WONMsg::DirG2FindDirectoryEx)))
	{
		WDBG_AH("SMsgDirG2FindDirectory::Unpack Not a DirG2FindDirectory(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2FindDirectory(Ex) message.");
	}

	mEntitiesPerReply = ReadShort();
	UnpackKey(*this);
	ReadWString(mDirName);
	ReadWString(mDisplayName);
	UnpackSearchObjects();
	UnpackExtended();
}
