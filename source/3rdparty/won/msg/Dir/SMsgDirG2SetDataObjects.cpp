// SMsgDirG2SetDataObjects

// DirectoryServer set data objects message.  Updates the set of data
// objects attached to an entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2SetDataObjects.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateExtendBase;
	using WONMsg::SMsgDirG2SetDataObjects;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2SetDataObjects::SMsgDirG2SetDataObjects(KeyType theType) :
	SMsgDirG2UpdateExtendBase(theType, true, false),
	mSetMode(WONMsg::DOSM_ADDREPLACE)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceSetDataObjects : WONMsg::DirG2DirectorySetDataObjects);
}


// SmallMessage ctor
SMsgDirG2SetDataObjects::SMsgDirG2SetDataObjects(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mSetMode(WONMsg::DOSM_ADDREPLACE)
{
	Unpack();
}


// Copy ctor
SMsgDirG2SetDataObjects::SMsgDirG2SetDataObjects(const SMsgDirG2SetDataObjects& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mSetMode(theMsgR.mSetMode)
{}


// Destructor
SMsgDirG2SetDataObjects::~SMsgDirG2SetDataObjects(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2SetDataObjects&
SMsgDirG2SetDataObjects::operator=(const SMsgDirG2SetDataObjects& theMsgR)
{
	SMsgDirG2UpdateExtendBase::operator=(theMsgR);
	mSetMode = theMsgR.mSetMode;
	return *this;
}


// SMsgDirG2SetDataObjects::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2SetDataObjects::Pack(void)
{
	WTRACE("SMsgDirG2SetDataObjects::Pack");
	SetExtended(true, false);
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceSetDataObjects : WONMsg::DirG2DirectorySetDataObjects);
	SMsgDirG2UpdateExtendBase::Pack();

	AppendByte(static_cast<unsigned char>(mSetMode));
	PackKey(*this);
	PackExtended();
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2SetDataObjects::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2SetDataObjects::Unpack(void)
{
	WTRACE("SMsgDirG2SetDataObjects::Unpack");
	mKeyType  = (GetMessageType() == WONMsg::DirG2ServiceSetDataObjects ? KT_SERVICE : KT_DIRECTORY);
	SetExtended(true, false);
	SMsgDirG2UpdateExtendBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2DirectorySetDataObjects) &&
	     (GetMessageType() != WONMsg::DirG2ServiceSetDataObjects)))
	{
		WDBG_AH("SMsgDirG2SetDataObjects::Unpack Not a DirG2SetDataObjects message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2SetDataObjects message.");
	}

	mSetMode = static_cast<WONMsg::DirG2DataObjectSetMode>(ReadByte());
	UnpackKey(*this);
	UnpackExtended();
	UnpackPeerData();
}
