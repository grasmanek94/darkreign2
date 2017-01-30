// SMsgDirG2ModifyDataObjects

// DirectoryServer modify data objects message.  Insert data or replace all or
// part of the data in the data field of specified data objects for specified entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2ModifyDataObjects.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2ModifyDataObjects;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2ModifyDataObjects::SMsgDirG2ModifyDataObjects(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mObjects()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceModifyDataObjects : WONMsg::DirG2DirectoryModifyDataObjects);
}


// SmallMessage ctor
SMsgDirG2ModifyDataObjects::SMsgDirG2ModifyDataObjects(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mObjects()
{
	Unpack();
}


// Copy ctor
SMsgDirG2ModifyDataObjects::SMsgDirG2ModifyDataObjects(const SMsgDirG2ModifyDataObjects& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mObjects(theMsgR.mObjects)
{}


// Destructor
SMsgDirG2ModifyDataObjects::~SMsgDirG2ModifyDataObjects(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2ModifyDataObjects&
SMsgDirG2ModifyDataObjects::operator=(const SMsgDirG2ModifyDataObjects& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mObjects = theMsgR.mObjects;
	return *this;
}


// SMsgDirG2ModifyDataObjects::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2ModifyDataObjects::Pack(void)
{
	WTRACE("SMsgDirG2ModifyDataObjects::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceModifyDataObjects : WONMsg::DirG2DirectoryModifyDataObjects);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);
	AppendShort(mObjects.size());

	ModifyDOInfoList::iterator anItr(mObjects.begin());
	for (; anItr != mObjects.end(); anItr++)
	{
		AppendShort(anItr->mOffset);
		AppendByte(anItr->mIsInsert ? 1 : 0);

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


// SMsgDirG2ModifyDataObjects::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2ModifyDataObjects::Unpack(void)
{
	WTRACE("SMsgDirG2ModifyDataObjects::Unpack");
	mObjects.clear();
	mKeyType = (GetMessageType() == WONMsg::DirG2ServiceModifyDataObjects ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2DirectoryModifyDataObjects) &&
	     (GetMessageType() != WONMsg::DirG2ServiceModifyDataObjects)))
	{
		WDBG_AH("SMsgDirG2ModifyDataObjects::Unpack Not a DirG2ModifyDataObjects message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2ModifyDataObjects message.");
	}

	UnpackKey(*this);
	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2ClearDataObjects::Unpack Reading " << aCt << " objects.");

	for (int i=0; i < aCt; i++)
	{
		ModifyDOInfo anObj;
		anObj.mOffset   = ReadShort();
		anObj.mIsInsert = (ReadByte() == 0 ? false : true);

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
