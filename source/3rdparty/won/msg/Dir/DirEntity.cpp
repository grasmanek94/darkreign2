// DirEntity

// Defines a generic entity used to represent nodes in the DirServer's tree.
// DirEntities may generically represent a directory node or service node.  This
// implies that not all fields of a DirEntity are applicable, the type of the
// DirEntity must be examined to determine which fields are appropriate.

// DirEntity is used internally in many DirServer messages to represent a
// DirServer node.  Depending on the message and it's attributes, the various
// fields of the DirEntity may or may not be filled in.  The context of the
// individual message must be examined in order to determine which fields of
// DirEntity will have meaning.


#include "common/won.h"
#include "msg/TMessage.h"
#include "DirEntity.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONAuth::Permission;
	using WONAuth::PermissionACL;
	using WONMsg::DirACLList;
	using WONMsg::BaseMessage;
	using WONMsg::DirEntity;
};


// Static members
const WONCommon::DataObjectTypeSet DirEntity::gEmptyTypeSet;


// ** Constructors / Destructor **


DirEntity::DirEntity() :
	mType(DE_UNKNOWN),
	mPath(),
	mName(),
	mDisplayName(),
	mCreated(0),
	mTouched(0),
	mCreateId(0),
	mTouchId(0),
	mLifespan(0),
	mCRC(0),
	mDataObjects(),
	mACLs(),
	mVisible(DE_UNKNOWN),
	mNetAddress()
{}


DirEntity::DirEntity(const DirEntity& theEntry) :
	mType(theEntry.mType),
	mPath(theEntry.mPath),
	mName(theEntry.mName),
	mDisplayName(theEntry.mDisplayName),
	mCreated(theEntry.mCreated),
	mTouched(theEntry.mTouched),
	mCreateId(theEntry.mCreateId),
	mTouchId(theEntry.mTouchId),
	mLifespan(theEntry.mLifespan),
	mCRC(theEntry.mCRC),
	mDataObjects(theEntry.mDataObjects),
	mACLs(theEntry.mACLs),
	mVisible(theEntry.mVisible),
	mNetAddress(theEntry.mNetAddress)
{}


DirEntity::~DirEntity()
{}


// ** Private Methods **

void
DirEntity::PackDataObjects(BaseMessage& theMsgR, const DataObjectTypeSet& theSetR,
                           unsigned long theFlags)
{
	WTRACE("DirEntity::PackDataObjects");
	WDBG_LL("DirEntity::PackDataObject Packing data objects, size=" << theSetR.size());
	theMsgR.AppendShort(theSetR.size());

	bool packType = ((theFlags & WONMsg::GF_ADDDOTYPE) != 0);
	bool packData = ((theFlags & WONMsg::GF_ADDDODATA) != 0);
	DataObjectTypeSet::const_iterator anItr(theSetR.begin());
	for (; anItr != theSetR.end(); anItr++)
	{
		if (packType)
		{
			unsigned char aTypeLen = anItr->GetDataType().size();
			theMsgR.AppendByte(aTypeLen);
			theMsgR.AppendBytes(aTypeLen, anItr->GetDataType().data());
		}

		if (packData)
		{
			unsigned short aDataLen = anItr->GetData().size();
			theMsgR.AppendShort(aDataLen);
			theMsgR.AppendBytes(aDataLen, anItr->GetData().data());
		}
	}
}


void
DirEntity::UnpackDataObjects(BaseMessage& theMsgR, DataObjectTypeSet& theSetR,
                             unsigned long theFlags)
{
	WTRACE("DirEntity::UnpackDataObjects");
	unsigned short aCt = theMsgR.ReadShort();
	WDBG_LL("DirEntity::UnpackDataObjects Unpacking data objects, size=" << aCt);
	theSetR.clear();

	bool unpackType = ((theFlags & WONMsg::GF_ADDDOTYPE) != 0);
	bool unpackData = ((theFlags & WONMsg::GF_ADDDODATA) != 0);
	for (int i=0; i < aCt; i++)
	{
		DataObject anObj;
		if (unpackType)
		{
			unsigned char aLen = theMsgR.ReadByte();
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(theMsgR.ReadBytes(aLen)), aLen);
		}
		else
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(&i), sizeof(i));

		if (unpackData)
		{
			unsigned short aLen = theMsgR.ReadShort();
			anObj.GetData().assign(reinterpret_cast<const unsigned char*>(theMsgR.ReadBytes(aLen)), aLen);
		}

		theSetR.insert(anObj);
	}
}


void
DirEntity::PackACLs(BaseMessage& theMsgR) const
{
	WTRACE("DirEntity::PackACLs");
	WDBG_LL("DirEntity::PackACLs Packing ACLs, size=" << mACLs.size());
	theMsgR.AppendShort(mACLs.size());

	DirACLList::const_iterator anACLItr(mACLs.begin());
	for (; anACLItr != mACLs.end(); anACLItr++)
	{
		theMsgR.AppendByte(anACLItr->mType);
		theMsgR.AppendShort(anACLItr->mACL.size());
		PermissionACL::const_iterator aPermItr(anACLItr->mACL.begin());
		for (; aPermItr != anACLItr->mACL.end(); aPermItr++)
		{
			theMsgR.AppendLong(aPermItr->mUserId);
			theMsgR.AppendLong(aPermItr->mCommunityId);
			theMsgR.AppendShort(aPermItr->mTrustLevel);
		}
	}
}


void
DirEntity::UnpackACLs(BaseMessage& theMsgR)
{
	WTRACE("DirEntity::UnpackACLs");
	unsigned short anACLCt = theMsgR.ReadShort();
	WDBG_LL("DirEntity::UnpackACLs Unpacking ACLs, size=" << anACLCt);
	mACLs.clear();

	for (int i=0; i < anACLCt; i++)
	{
		DirACL anACL;
		anACL.mType = static_cast<WONMsg::DirG2ACLType>(theMsgR.ReadByte());

		unsigned short aPermCt = theMsgR.ReadShort();
		for (int j=0; j < aPermCt; j++)
		{
			Permission aPerm;
			aPerm.mUserId      = theMsgR.ReadLong();
			aPerm.mCommunityId = theMsgR.ReadLong();
			aPerm.mTrustLevel  = theMsgR.ReadShort();
			anACL.mACL.insert(aPerm);
		}

		mACLs.push_back(anACL);
	}
}


// ** Public methods **

DirEntity&
DirEntity::operator=(const DirEntity& theEntry)
{
	if (this != &theEntry)
	{
		mType         = theEntry.mType;
		mPath         = theEntry.mPath;
		mName         = theEntry.mName;
		mDisplayName  = theEntry.mDisplayName;
		mCreated      = theEntry.mCreated;
		mTouched      = theEntry.mTouched;
		mCreateId     = theEntry.mCreateId;
		mTouchId      = theEntry.mTouchId;
		mLifespan     = theEntry.mLifespan;
		mCRC          = theEntry.mCRC;
		mDataObjects  = theEntry.mDataObjects;
		mACLs         = theEntry.mACLs;
		mVisible      = theEntry.mVisible;
		mNetAddress   = theEntry.mNetAddress;
	}
	return *this;
}


const std::wstring
DirEntity::GetFullPath() const
{
	wstring aRet(mPath);
	if (mType == ET_DIRECTORY)
	{
		if ((! aRet.empty()) && (aRet[aRet.size()-1] != L'/'))
			aRet += L'/';
		aRet += mName;
	}
	return aRet;
}


void
DirEntity::Pack(BaseMessage& theMsgR, unsigned long theGetFlags,
                const DataObjectTypeSet& theSetR) const
{
	WTRACE("DirEntity::Pack");
	WDBG_LL("DirEntity::Pack Packing entity, type=" << mType);

	// Add type if needed
	if (theGetFlags & WONMsg::GF_ADDTYPE)
		theMsgR.AppendByte(mType);

	// Entity specific fields
	switch (mType)
	{
	case DirEntity::ET_DIRECTORY:
		// Add dir path if needed
		if (theGetFlags & WONMsg::GF_DIRADDPATH)
			theMsgR.Append_PW_STRING(mPath);
		// Add dir name if needed
		if (theGetFlags & WONMsg::GF_DIRADDNAME)
			theMsgR.Append_PW_STRING(mName);
		// Add dir visibility if needed
		if (theGetFlags & WONMsg::GF_DIRADDVISIBLE)
			theMsgR.AppendByte(mVisible);
		break;
	case DirEntity::ET_SERVICE:
		// Add serv path if needed
		if (theGetFlags & WONMsg::GF_SERVADDPATH)
			theMsgR.Append_PW_STRING(mPath);
		// Add serv name if needed
		if (theGetFlags & WONMsg::GF_SERVADDNAME)
			theMsgR.Append_PW_STRING(mName);
		// Add serv netaddress if needed
		if (theGetFlags & WONMsg::GF_SERVADDNETADDR)
		{
			theMsgR.AppendByte(mNetAddress.size());
			theMsgR.AppendBytes(mNetAddress.size(), mNetAddress.data());
		}
		break;
	}

	// Add displayName if needed
	if (theGetFlags & WONMsg::GF_ADDDISPLAYNAME)
		theMsgR.Append_PW_STRING(mDisplayName);

	// Add lifespan if needed
	if (theGetFlags & WONMsg::GF_ADDLIFESPAN)
		theMsgR.AppendLong(mLifespan);

	// Add create date if needed
	if (theGetFlags & WONMsg::GF_ADDCREATED)
		theMsgR.AppendLong(mCreated);

	// Add touched date if needed
	if (theGetFlags & WONMsg::GF_ADDTOUCHED)
		theMsgR.AppendLong(mTouched);

	// Add CRC if needed
	if (theGetFlags & WONMsg::GF_ADDCRC)
		theMsgR.AppendLong(mCRC);

	// Add UserIds if needed
	if (theGetFlags & WONMsg::GF_ADDUIDS)
	{
		theMsgR.AppendLong(mCreateId);
		theMsgR.AppendLong(mTouchId);
	}

	// Add all data objects if needed
	if (theGetFlags & WONMsg::GF_ADDDATAOBJECTS)
		PackDataObjects(theMsgR, mDataObjects, theGetFlags);

	// Otherwise, add requested data objects (if any)
	else if (theSetR.size() > 0)
	{
		DataObjectTypeSet aPackSet;
		DataObjectTypeSet::const_iterator anItr(theSetR.begin());
		for (; anItr != theSetR.end(); anItr++)
		{
			DataObjectTypeSet::const_iterator aSrch(mDataObjects.find(*anItr));
			if (aSrch != mDataObjects.end())
				aPackSet.insert(*aSrch);
		}
		PackDataObjects(theMsgR, aPackSet, theGetFlags);
	}

	// Add ACLs if needed (deferred)
	if (theGetFlags & WONMsg::GF_ADDACLS)
		PackACLs(theMsgR);
}


void
DirEntity::Unpack(BaseMessage& theMsgR, unsigned long theGetFlags)
{
	WTRACE("DirEntity::Unpack");
	WDBG_LL("DirEntity::Unpack Unpacking entity");

	// Read type if needed
	if (theGetFlags & WONMsg::GF_ADDTYPE)
		mType = theMsgR.ReadByte();

	// Entity specific fields
	switch (mType)
	{
	case ET_DIRECTORY:
		// Read dir path if needed
		if (theGetFlags & WONMsg::GF_DIRADDPATH)
			theMsgR.ReadWString(mPath);
		//Read dir name if needed
		if (theGetFlags & WONMsg::GF_DIRADDNAME)
			theMsgR.ReadWString(mName);
		// Read dir visibility if needed
		if (theGetFlags & WONMsg::GF_DIRADDVISIBLE)
			mVisible = theMsgR.ReadByte();
		break;
	case ET_SERVICE:
		// Read serv path if needed
		if (theGetFlags & WONMsg::GF_SERVADDPATH)
			theMsgR.ReadWString(mPath);
		// Read serv name if needed
		if (theGetFlags & WONMsg::GF_SERVADDNAME)
			theMsgR.ReadWString(mName);
		// Read serv netaddress if needed
		if (theGetFlags & WONMsg::GF_SERVADDNETADDR)
		{
			unsigned char aCt = theMsgR.ReadByte();
			mNetAddress.assign(reinterpret_cast<const unsigned char*>(theMsgR.ReadBytes(aCt)), aCt);
		}
		break;
	}

	// Read displayName if needed
	if (theGetFlags & WONMsg::GF_ADDDISPLAYNAME)
		theMsgR.ReadWString(mDisplayName);

	// Read lifespan if needed
	if (theGetFlags & WONMsg::GF_ADDLIFESPAN)
		mLifespan = theMsgR.ReadLong();

	// Read create date if needed
	if (theGetFlags & WONMsg::GF_ADDCREATED)
		mCreated = theMsgR.ReadLong();

	// Read touched date if needed
	if (theGetFlags & WONMsg::GF_ADDTOUCHED)
		mTouched = theMsgR.ReadLong();

	// Read CRC if needed
	if (theGetFlags & WONMsg::GF_ADDCRC)
		mCRC = theMsgR.ReadLong();

	// Add UserIds if needed
	if (theGetFlags & WONMsg::GF_ADDUIDS)
	{
		mCreateId = theMsgR.ReadLong();
		mTouchId  = theMsgR.ReadLong();
	}

	// Read Data Objects if needed
	if (theGetFlags & WONMsg::GF_ADDDATAOBJECTS)
		UnpackDataObjects(theMsgR, mDataObjects, theGetFlags);

	// Read ACLs if needed (deferred)
	if (theGetFlags & WONMsg::GF_ADDACLS)
		UnpackACLs(theMsgR);
}


unsigned long
DirEntity::ComputeSize(unsigned long theGetFlags, const DataObjectTypeSet& theSetR) const
{
	unsigned long aSize = 0;

	// Add type if needed
	if (theGetFlags & WONMsg::GF_ADDTYPE)
		aSize += sizeof(mType);

	// Entity specific fields
	switch (mType)
	{
	case DirEntity::ET_DIRECTORY:
		// Add dir path if needed
		if (theGetFlags & WONMsg::GF_DIRADDPATH)
			aSize += TRawMsg::ComputeWStringPackSize(mPath);
		// Add dir name if needed
		if (theGetFlags & WONMsg::GF_DIRADDNAME)
			aSize += TRawMsg::ComputeWStringPackSize(mName);
		// Add dir visibility if needed
		if (theGetFlags & WONMsg::GF_DIRADDVISIBLE)
			aSize += sizeof(mVisible);
		break;
	case DirEntity::ET_SERVICE:
		// Add serv path if needed
		if (theGetFlags & WONMsg::GF_SERVADDPATH)
			aSize += TRawMsg::ComputeWStringPackSize(mPath);
		// Add serv name if needed
		if (theGetFlags & WONMsg::GF_SERVADDNAME)
			aSize += TRawMsg::ComputeWStringPackSize(mName);
		// Add serv netaddress if needed
		if (theGetFlags & WONMsg::GF_SERVADDNETADDR)
			aSize += sizeof(unsigned char) + mNetAddress.size();
		break;
	}

	// Add displayName if needed
	if (theGetFlags & WONMsg::GF_ADDDISPLAYNAME)
		aSize += TRawMsg::ComputeWStringPackSize(mDisplayName);

	// Add lifespan if needed
	if (theGetFlags & WONMsg::GF_ADDLIFESPAN)
		aSize += sizeof(mLifespan);

	// Add create date if needed
	if (theGetFlags & WONMsg::GF_ADDCREATED)
		aSize += sizeof(mCreated);

	// Add touched date if needed
	if (theGetFlags & WONMsg::GF_ADDTOUCHED)
		aSize += sizeof(mTouched);

	// Add CRC if needed
	if (theGetFlags & WONMsg::GF_ADDCRC)
		aSize += sizeof(mCRC);

	// Add UserIds if needed
	if (theGetFlags & WONMsg::GF_ADDUIDS)
		aSize += sizeof(mCreateId) + sizeof(mTouchId);

	// Data Objects
	bool packAllObjs = ((theGetFlags & WONMsg::GF_ADDDATAOBJECTS) != 0);
	if ((packAllObjs) || (! theSetR.empty()))
	{
		aSize += sizeof(unsigned short);  // data object count
		bool packType = ((theGetFlags & WONMsg::GF_ADDDOTYPE) != 0);
		bool packData = ((theGetFlags & WONMsg::GF_ADDDODATA) != 0);
		DataObjectTypeSet::const_iterator anItr(mDataObjects.begin());
		for (; anItr != mDataObjects.end(); ++anItr)
		{
			if ((packAllObjs) || (theSetR.find(*anItr) != theSetR.end()))
			{
				if (packType)
					aSize += sizeof(unsigned char)  + anItr->GetDataType().size();
				if (packData)
					aSize += sizeof(unsigned short) + anItr->GetData().size();
			}
		}
	}

	// Add ACLs if needed
	if (theGetFlags & WONMsg::GF_ADDACLS)
	{
		aSize += sizeof(unsigned short);  // ACL count
		DirACLList::const_iterator anItr(mACLs.begin());
		for (; anItr != mACLs.end(); ++anItr)
			aSize += sizeof(unsigned char) + sizeof(unsigned short) + (anItr->mACL.size() * sizeof(Permission));
	}

	return aSize;
}


