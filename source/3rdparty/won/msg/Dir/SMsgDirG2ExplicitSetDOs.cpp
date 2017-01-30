// SMsgDirG2ExplicitSetDataObjects

// DirectoryServer explicit set data objects message.  Updates the set of data objects
// attached to an entity with a specified set of data objects.  Set mode is specified
// for each data object in the update set.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2ExplicitSetDOs.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2ExplicitSetDataObjects;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2ExplicitSetDataObjects::SMsgDirG2ExplicitSetDataObjects(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mObjects()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceExplicitSetDataObjects : WONMsg::DirG2DirectoryExplicitSetDataObjects);
}


// SmallMessage ctor
SMsgDirG2ExplicitSetDataObjects::SMsgDirG2ExplicitSetDataObjects(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mObjects()
{
	Unpack();
}


// Copy ctor
SMsgDirG2ExplicitSetDataObjects::SMsgDirG2ExplicitSetDataObjects(const SMsgDirG2ExplicitSetDataObjects& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mObjects(theMsgR.mObjects)
{}


// Destructor
SMsgDirG2ExplicitSetDataObjects::~SMsgDirG2ExplicitSetDataObjects(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2ExplicitSetDataObjects&
SMsgDirG2ExplicitSetDataObjects::operator=(const SMsgDirG2ExplicitSetDataObjects& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mObjects = theMsgR.mObjects;
	return *this;
}


// SMsgDirG2ExplicitSetDataObjects::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2ExplicitSetDataObjects::Pack(void)
{
	WTRACE("SMsgDirG2ExplicitSetDataObjects::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceExplicitSetDataObjects : WONMsg::DirG2DirectoryExplicitSetDataObjects);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);

	AppendShort(mObjects.size());
	ExplicitDOInfoList::iterator anItr(mObjects.begin());
	for (; anItr != mObjects.end(); anItr++)
	{
		AppendByte(static_cast<unsigned char>(anItr->mSetMode));

		unsigned char aTypeLen = anItr->mData.GetDataType().size();
		AppendByte(aTypeLen);
		if (aTypeLen > 0)
			AppendBytes(aTypeLen, anItr->mData.GetDataType().data());

		unsigned short aDataLen = anItr->mData.GetData().size();
		AppendShort(aDataLen);
		if (aDataLen > 0)
			AppendBytes(aDataLen, anItr->mData.GetData().data());
	}

	PackPeerData();
	return GetDataPtr();
}


// SMsgDirG2ExplicitSetDataObjects::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2ExplicitSetDataObjects::Unpack(void)
{
	WTRACE("SMsgDirG2ExplicitSetDataObjects::Unpack");
	mObjects.clear();
	mKeyType = (GetMessageType() == WONMsg::DirG2ServiceExplicitSetDataObjects ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2DirectoryExplicitSetDataObjects) &&
	     (GetMessageType() != WONMsg::DirG2ServiceExplicitSetDataObjects)))
	{
		WDBG_AH("SMsgDirG2ExplicitSetDataObjects::Unpack Not a DirG2ExplicitSetDataObjects message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2ExplicitSetDataObjects message.");
	}

	UnpackKey(*this);
	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2ExplicitSetDataObjects::Unpack Reading " << aCt << " objects.");

	for (int i=0; i < aCt; i++)
	{
		ExplicitDOInfo anObj;
		anObj.mSetMode = static_cast<WONMsg::DirG2DataObjectSetMode>(ReadByte());

		unsigned char aTypeLen = ReadByte();
		if (aTypeLen > 0)
			anObj.mData.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aTypeLen)), aTypeLen);

		unsigned short aDataLen = ReadShort();
		if (aDataLen > 0)
			anObj.mData.GetData().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataLen)), aDataLen);

		mObjects.push_back(anObj);
	}

	UnpackPeerData();
}
