// SMsgDirG2RemoveEntity.h

// DirectoryServer remove entity message.  Removes a specified entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2RemoveEntity.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2RemoveEntity;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2RemoveEntity::SMsgDirG2RemoveEntity(KeyType theType) :
	SMsgDirG2UpdateBase(theType)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2RemoveService : WONMsg::DirG2RemoveDirectory);
}


// SmallMessage ctor
SMsgDirG2RemoveEntity::SMsgDirG2RemoveEntity(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR)
{
	Unpack();
}


// Copy ctor
SMsgDirG2RemoveEntity::SMsgDirG2RemoveEntity(const SMsgDirG2RemoveEntity& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR)
{}


// Destructor
SMsgDirG2RemoveEntity::~SMsgDirG2RemoveEntity(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2RemoveEntity&
SMsgDirG2RemoveEntity::operator=(const SMsgDirG2RemoveEntity& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	return *this;
}


// SMsgDirG2RemoveEntity::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2RemoveEntity::Pack(void)
{
	WTRACE("SMsgDirG2RemoveEntity::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2RemoveService : WONMsg::DirG2RemoveDirectory);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2RemoveEntity::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2RemoveEntity::Unpack(void)
{
	WTRACE("SMsgDirG2RemoveEntity::Unpack");
	mKeyType = (GetMessageType() == WONMsg::DirG2RemoveService ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2RemoveDirectory) &&
	     (GetMessageType() != WONMsg::DirG2RemoveService)))
	{
		WDBG_AH("SMsgDirG2RemoveEntity::Unpack Not a DirG2RemoveEntity message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2RemoveEntity message.");
	}

	UnpackKey(*this);
	UnpackPeerData();
}
