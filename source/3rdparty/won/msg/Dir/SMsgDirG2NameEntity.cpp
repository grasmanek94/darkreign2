// SMsgDirG2NameEntity

// DirectoryServer name entity message.  Updates the display name for a specified
// entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2NameEntity.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2NameEntity;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2NameEntity::SMsgDirG2NameEntity(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mDisplayName(),
	mUnique(false)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2NameService : WONMsg::DirG2NameDirectory);
}


// SmallMessage ctor
SMsgDirG2NameEntity::SMsgDirG2NameEntity(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mDisplayName(),
	mUnique(false)
{
	Unpack();
}


// Copy ctor
SMsgDirG2NameEntity::SMsgDirG2NameEntity(const SMsgDirG2NameEntity& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mDisplayName(theMsgR.mDisplayName),
	mUnique(theMsgR.mUnique)
{}


// Destructor
SMsgDirG2NameEntity::~SMsgDirG2NameEntity(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2NameEntity&
SMsgDirG2NameEntity::operator=(const SMsgDirG2NameEntity& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mDisplayName = theMsgR.mDisplayName;
	mUnique      = theMsgR.mUnique;
	return *this;
}


// SMsgDirG2NameEntity::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2NameEntity::Pack(void)
{
	WTRACE("SMsgDirG2NameEntity::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2NameService : WONMsg::DirG2NameDirectory);
	SMsgDirG2UpdateBase::Pack();

	AppendByte(mUnique ? 1 : 0);
	PackKey(*this);
	Append_PW_STRING(mDisplayName);
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2NameEntity::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2NameEntity::Unpack(void)
{
	WTRACE("SMsgDirG2NameEntity::Unpack");
	mKeyType = (GetMessageType() == WONMsg::DirG2NameService ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2NameDirectory) &&
	     (GetMessageType() != WONMsg::DirG2NameService)))
	{
		WDBG_AH("SMsgDirG2NameEntity::Unpack Not a DirG2NameEntity message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2NameEntity message.");
	}

	mUnique = (ReadByte() == 0 ? false : true);
	UnpackKey(*this);
	ReadWString(mDisplayName);
	UnpackPeerData();
}
