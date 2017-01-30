// SMsgDirG2GetEntity

// DirectoryServer get entity message.  Requests a directory/service and or its contents.
// Fields in reply are configurable.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "DirEntity.h"
#include "SMsgDirG2QueryGetBase.h"
#include "SMsgDirG2GetEntity.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryGetBase;
	using WONMsg::SMsgDirG2GetEntity;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2GetEntity::SMsgDirG2GetEntity(KeyType theType, bool isExtended) :
	SMsgDirG2QueryGetBase(theType, isExtended)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	if (mKeyType == KT_SERVICE)
		SetMessageType(mExtended ? WONMsg::DirG2GetServiceEx : WONMsg::DirG2GetService);
	else
		SetMessageType(mExtended ? WONMsg::DirG2GetDirectoryEx : WONMsg::DirG2GetDirectory);
}


// SmallMessage ctor
SMsgDirG2GetEntity::SMsgDirG2GetEntity(const SmallMessage& theMsgR) :
	SMsgDirG2QueryGetBase(theMsgR)
{
	Unpack();
}


// Copy ctor
SMsgDirG2GetEntity::SMsgDirG2GetEntity(const SMsgDirG2GetEntity& theMsgR) :
	SMsgDirG2QueryGetBase(theMsgR)
{}


// Destructor
SMsgDirG2GetEntity::~SMsgDirG2GetEntity(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2GetEntity&
SMsgDirG2GetEntity::operator=(const SMsgDirG2GetEntity& theMsgR)
{
	SMsgDirG2QueryGetBase::operator=(theMsgR);
	return *this;
}


// SMsgDirG2GetEntity::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2GetEntity::Pack(void)
{
	WTRACE("SMsgDirG2GetEntity::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	if (mKeyType == KT_SERVICE)
		SetMessageType(mExtended ? WONMsg::DirG2GetServiceEx : WONMsg::DirG2GetService);
	else
		SetMessageType(mExtended ? WONMsg::DirG2GetDirectoryEx : WONMsg::DirG2GetDirectory);

	SMsgDirG2QueryGetBase::Pack();
	if (mKeyType != KT_SERVICE)
		AppendShort(mEntitiesPerReply);

	PackKey(*this);
	PackExtended();
	return GetDataPtr();
}


// SMsgDirG2GetEntity::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2GetEntity::Unpack(void)
{
	WTRACE("SMsgDirG2GetEntity::Unpack");
	bool validMsg = (GetServiceType() == WONMsg::SmallDirServerG2);
	if (validMsg)
	{
		switch (GetMessageType())
		{
		case WONMsg::DirG2GetDirectory:
			SetKeyType(KT_DIRECTORY);
			mExtended = false;
			break;
		case WONMsg::DirG2GetDirectoryEx:
			SetKeyType(KT_DIRECTORY);
			mExtended = true;  break;
		case WONMsg::DirG2GetService:
			SetKeyType(KT_SERVICE);
			mExtended = false;
			break;
		case WONMsg::DirG2GetServiceEx:
			SetKeyType(KT_SERVICE);
			mExtended = true;
			break;
		default:
			mExtended = validMsg = false;
			break;
		}
	}

	SMsgDirG2QueryGetBase::Unpack();
	if (! validMsg)
	{
		WDBG_AH("SMsgDirG2GetEntity::Unpack Not a DirG2GetEntity(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2GetEntity(Ex) message.");
	}

	mEntitiesPerReply = (mKeyType != KT_SERVICE ? ReadShort() : 0);
	UnpackKey(*this);
	UnpackExtended();
}
