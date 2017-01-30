// SMsgDirG2RenewEntity

// DirectoryServer renew entity message.  Updates the lifespan for a specified
// entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2RenewEntity.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2RenewEntity;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2RenewEntity::SMsgDirG2RenewEntity(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mLifespan(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2RenewService : WONMsg::DirG2RenewDirectory);
}


// SmallMessage ctor
SMsgDirG2RenewEntity::SMsgDirG2RenewEntity(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2RenewEntity::SMsgDirG2RenewEntity(const SMsgDirG2RenewEntity& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
SMsgDirG2RenewEntity::~SMsgDirG2RenewEntity(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2RenewEntity&
SMsgDirG2RenewEntity::operator=(const SMsgDirG2RenewEntity& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mLifespan = theMsgR.mLifespan;
	return *this;
}


// SMsgDirG2RenewEntity::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2RenewEntity::Pack(void)
{
	WTRACE("SMsgDirG2RenewEntity::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2RenewService : WONMsg::DirG2RenewDirectory);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);
	AppendLong(mLifespan);
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2RenewEntity::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2RenewEntity::Unpack(void)
{
	WTRACE("SMsgDirG2RenewEntity::Unpack");
	mKeyType = (GetMessageType() == WONMsg::DirG2RenewService ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2RenewDirectory) &&
	     (GetMessageType() != WONMsg::DirG2RenewService)))
	{
		WDBG_AH("SMsgDirG2RenewEntity::Unpack Not a DirG2RenewEntity message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2RenewEntity message.");
	}

	UnpackKey(*this);
	mLifespan = ReadLong();
	UnpackPeerData();
}
