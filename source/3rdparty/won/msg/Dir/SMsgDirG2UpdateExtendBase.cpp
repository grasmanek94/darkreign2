// SMsgDirG2UpdateExtendBase.h

// Base class derived from UpdateBase that provides common definitions for extended
// updates. Adds list of data objects and list of update ACLs.  Also adds logical
// members to enable/disable packing of data obejcts and ACLs.

// The enableDataObjects and enableACLs members control whether they data objects and
// ACL members respectively are packed or unpacked in the PackExtended and UnpackExtended
// methods.  Use these members to enable/disable the data obejcts and/or ACLs.

// Adds a PackExtended() and UnpackExtended() methods to pack/unpack the data objects
// and ACLs.  These methods should be called be derived class Pack/Unpack at the
// appropriate point to pack/unpack extended data if needed.


#include "common/won.h"
#include "auth/Permission.h"
#include "msg/TMessage.h"
#include "DirEntity.h"
#include "SMsgDirG2UpdateExtendBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONAuth::Permission;
	using WONAuth::PermissionACL;
	using WONMsg::DirACL;
	using WONMsg::DirACLList;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2UpdateExtendBase;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2UpdateExtendBase::SMsgDirG2UpdateExtendBase(KeyType theType, bool enableDataObjects,
                                                     bool enableACLs) :
	SMsgDirG2UpdateBase(theType),
	mDataObjects(),
	mACLs(),
	mEnableDataObjects(enableDataObjects),
	mEnableACLs(enableACLs)
{}


// SmallMessage ctor
SMsgDirG2UpdateExtendBase::SMsgDirG2UpdateExtendBase(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mDataObjects(),
	mACLs(),
	mEnableDataObjects(false),
	mEnableACLs(false)
{}


// Copy ctor
SMsgDirG2UpdateExtendBase::SMsgDirG2UpdateExtendBase(const SMsgDirG2UpdateExtendBase& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mDataObjects(theMsgR.mDataObjects),
	mACLs(theMsgR.mACLs),
	mEnableDataObjects(theMsgR.mEnableDataObjects),
	mEnableACLs(theMsgR.mEnableACLs)
{}


// Destructor
SMsgDirG2UpdateExtendBase::~SMsgDirG2UpdateExtendBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2UpdateExtendBase::PackExtended(void)
{
	WTRACE("SMsgDirG2UpdateExtendBase::PackExtended");
	WDBG_LL("SMsgDirG2UpdateExtendBase::PackExtended enableDataObjects=" << mEnableDataObjects << " enableACLs=" << mEnableACLs);

	if (mEnableDataObjects)
	{
		WDBG_LL("SMsgDirG2UpdateExtendBase::PackExtended Writing " << mDataObjects.size() << " data objects.");
		AppendShort(mDataObjects.size());
		DataObjectTypeSet::iterator anItr(mDataObjects.begin());
		for (; anItr != mDataObjects.end(); anItr++)
		{
			unsigned char aTypeLen = anItr->GetDataType().size();
			AppendByte(aTypeLen);
			if (aTypeLen > 0)
				AppendBytes(aTypeLen, anItr->GetDataType().data());

			unsigned short aDataLen = anItr->GetData().size();
			AppendShort(aDataLen);
			if (aDataLen > 0)
				AppendBytes(aDataLen, anItr->GetData().data());
		}
	}

	if (mEnableACLs)
	{
		WDBG_LL("SMsgDirG2UpdateExtendBase::PackExtended Writing " << mACLs.size() << " ACLs.");
		AppendShort(mACLs.size());

		DirACLList::const_iterator anACLItr(mACLs.begin());
		for (; anACLItr != mACLs.end(); anACLItr++)
		{
			AppendByte(anACLItr->mType);
			AppendShort(anACLItr->mACL.size());
			PermissionACL::const_iterator aPermItr(anACLItr->mACL.begin());
			for (; aPermItr != anACLItr->mACL.end(); aPermItr++)
			{
				AppendLong(aPermItr->mUserId);
				AppendLong(aPermItr->mCommunityId);
				AppendShort(aPermItr->mTrustLevel);
			}
		}
	}
}


void
SMsgDirG2UpdateExtendBase::UnpackExtended(void)
{
	WTRACE("SMsgDirG2UpdateExtendBase::UnpackExtended");
	WDBG_LL("SMsgDirG2UpdateExtendBase::UnpackExtended enableDataObjects=" << mEnableDataObjects << " enableACLs=" << mEnableACLs);
	mDataObjects.clear();
	mACLs.clear();

	if (mEnableDataObjects)
	{
		unsigned short aCt = ReadShort();
		WDBG_LL("SMsgDirG2UpdateExtendBase::UnpackExtended Reading " << aCt << " data objects.");

		for (int i=0; i < aCt; i++)
		{
			DataObject anObj;
			unsigned char aTypeLen = ReadByte();
			if (aTypeLen > 0)
				anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aTypeLen)), aTypeLen);

			unsigned short aDataLen = ReadShort();
			if (aDataLen > 0)
				anObj.GetData().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataLen)), aDataLen);

			mDataObjects.insert(anObj);
		}
	}

	if (mEnableACLs)
	{
		unsigned short aCt = ReadShort();
		WDBG_LL("SMsgDirG2UpdateExtendBase::UnpackExtended Reading " << aCt << " ACLs.");

		for (int i=0; i < aCt; i++)
		{
			DirACL anACL;
			anACL.mType = static_cast<WONMsg::DirG2ACLType>(ReadByte());

			unsigned short aPermCt = ReadShort();
			for (int j=0; j < aPermCt; j++)
			{
				Permission aPerm;
				aPerm.mUserId      = ReadLong();
				aPerm.mCommunityId = ReadLong();
				aPerm.mTrustLevel  = ReadShort();
				anACL.mACL.insert(aPerm);
			}

			mACLs.push_back(anACL);
		}
	}
}


// ** Public Methods **

// Assignment operator
SMsgDirG2UpdateExtendBase&
SMsgDirG2UpdateExtendBase::operator=(const SMsgDirG2UpdateExtendBase& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mDataObjects       = theMsgR.mDataObjects;
	mACLs              = theMsgR.mACLs;
	mEnableDataObjects = theMsgR.mEnableDataObjects;
	mEnableACLs        = theMsgR.mEnableACLs;
	return *this;
}
