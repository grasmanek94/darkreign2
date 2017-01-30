// SMsgDirG2ClearDataObjects

// DirectoryServer clear data objects message.  Removes specified data
// objects from the set of data objects attached to an entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2ClearDataObjects.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2ClearDataObjects;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2ClearDataObjects::SMsgDirG2ClearDataObjects(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mClearTypes()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceClearDataObjects : WONMsg::DirG2DirectoryClearDataObjects);
}


// SmallMessage ctor
SMsgDirG2ClearDataObjects::SMsgDirG2ClearDataObjects(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mClearTypes()
{
	Unpack();
}


// Copy ctor
SMsgDirG2ClearDataObjects::SMsgDirG2ClearDataObjects(const SMsgDirG2ClearDataObjects& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mClearTypes(theMsgR.mClearTypes)
{}


// Destructor
SMsgDirG2ClearDataObjects::~SMsgDirG2ClearDataObjects(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2ClearDataObjects&
SMsgDirG2ClearDataObjects::operator=(const SMsgDirG2ClearDataObjects& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mClearTypes = theMsgR.mClearTypes;
	return *this;
}


// SMsgDirG2ClearDataObjects::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2ClearDataObjects::Pack(void)
{
	WTRACE("SMsgDirG2ClearDataObjects::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceClearDataObjects : WONMsg::DirG2DirectoryClearDataObjects);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);

	AppendShort(mClearTypes.size());
	DataObjectTypeSet::iterator anItr(mClearTypes.begin());
	for (; anItr != mClearTypes.end(); anItr++)
	{
		unsigned char aLen = anItr->GetDataType().size();
		AppendByte(aLen);
		if (aLen > 0)
			AppendBytes(aLen, anItr->GetDataType().data());
	}

	PackPeerData();
	return GetDataPtr();
}


// SMsgDirG2ClearDataObjects::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2ClearDataObjects::Unpack(void)
{
	WTRACE("SMsgDirG2ClearDataObjects::Unpack");
	mClearTypes.clear();
	mKeyType = (GetMessageType() == WONMsg::DirG2ServiceClearDataObjects ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2DirectoryClearDataObjects) &&
	     (GetMessageType() != WONMsg::DirG2ServiceClearDataObjects)))
	{
		WDBG_AH("SMsgDirG2ClearDataObjects::Unpack Not a DirG2ClearDataObjects message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2ClearDataObjects message.");
	}

	UnpackKey(*this);
	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2ClearDataObjects::Unpack Reading " << aCt << " entries.");

	for (int i=0; i < aCt; i++)
	{
		DataObject anObj;
		unsigned char aLen = ReadByte();
		if (aLen > 0)
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aLen)), aLen);
		mClearTypes.insert(anObj);
	}

	UnpackPeerData();
}
