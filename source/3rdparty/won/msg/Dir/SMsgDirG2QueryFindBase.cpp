// SMsgDirG2QueryFindBase

// Base class derived from SMsgDirG2QueryGetBase that provides common definitions
// for find queries. Adds fields for MatchMode, FindFlags, and additional extended
// fields for search data types (extended finds only).

// Derived class should set the mExtended member from SMsgDirG2QueryExtendBase as
// needed to specify whether query is extended or not.  This should be done in
// constructors and Unpack.  Note that mExtended defaults to false.

// Note that the Pack() and Unpack() methods append/write the GetFlags (from base class),
// MatchMode, and FindFlags.  This implies that all classes derived from QueryFindBase
// MUST have getflags, MatchMode, and FindFlags as the first fields after their message
// header.

// Adds a PackSearchObjects() and UnpackSearchObjects() methods to pack/unpack the
// search data objects.  These methods should be called be derived class Pack/Unpack at
// the appropriate point to pack/unpack search objects if needed.  Note that these
// methods are NoOps if message is not extended (mExtended == false).

// Note that SMsgDirG2QueryFindBase adds an mDisplayName member, but DOES NOT Pack/Unpack
// this member automatically.  It is up to derived classes to pack/unpack displayName
// if needed.

// Note: SMsgDirG2QueryFindBase does not override the pure virtual Duplicate() method
// from SMsgDirG2QueryGetBase.  Derrived classes must override this method.



#include "common/won.h"
#include "msg/TMessage.h"
#include "SMsgDirG2QueryFindBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryGetBase;
	using WONMsg::SMsgDirG2QueryFindBase;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2QueryFindBase::SMsgDirG2QueryFindBase(KeyType theType, bool isExtended) :
	SMsgDirG2QueryGetBase(theType, isExtended),
	mFindMode(WONMsg::FMM_EXACT),
	mFindFlags(WONMsg::FF_NOFLAGS),
	mDisplayName(),
	mSearchObjects()
{}


// SmallMessage ctor
SMsgDirG2QueryFindBase::SMsgDirG2QueryFindBase(const SmallMessage& theMsgR) :
	SMsgDirG2QueryGetBase(theMsgR),
	mFindMode(WONMsg::FMM_EXACT),
	mFindFlags(WONMsg::FF_NOFLAGS),
	mDisplayName(),
	mSearchObjects()
{}


// Copy ctor
SMsgDirG2QueryFindBase::SMsgDirG2QueryFindBase(const SMsgDirG2QueryFindBase& theMsgR) :
	SMsgDirG2QueryGetBase(theMsgR),
	mFindMode(theMsgR.mFindMode),
	mFindFlags(theMsgR.mFindFlags),
	mDisplayName(theMsgR.mDisplayName),
	mSearchObjects(theMsgR.mSearchObjects)
{}


// Destructor
SMsgDirG2QueryFindBase::~SMsgDirG2QueryFindBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2QueryFindBase::PackSearchObjects(void)
{
	WTRACE("SMsgDirG2QueryFindBase::PackSearchObjects");
	WDBG_LL("SMsgDirG2QueryFindBase::PackSearchObjects Extended=" << mExtended << " SearchObjects size=" << mSearchObjects.size());
	if (! mExtended) return;

	AppendShort(mSearchObjects.size());
	DataObjectTypeSet::iterator anItr(mSearchObjects.begin());
	for (; anItr != mSearchObjects.end(); anItr++)
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


void
SMsgDirG2QueryFindBase::UnpackSearchObjects(void)
{
	WTRACE("SMsgDirG2QueryFindBase::UnpackSearchObjects");
	WDBG_LL("SMsgDirG2QueryFindBase::UnpackSearchObjects Extended=" << mExtended);
	mSearchObjects.clear();
	if (! mExtended) return;

	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2QueryFindBase::UnpackSearchObjects Reading " << aCt << " entries.");

	for (int i=0; i < aCt; i++)
	{
		DataObject anObj;
		unsigned char aTypeLen = ReadByte();
		if (aTypeLen > 0)
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aTypeLen)), aTypeLen);

		unsigned short aDataLen = ReadShort();
		if (aDataLen > 0)
			anObj.GetData().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataLen)), aDataLen);

		mSearchObjects.insert(anObj);
	}
}


// ** Public Methods **

// Assignment operator
SMsgDirG2QueryFindBase&
SMsgDirG2QueryFindBase::operator=(const SMsgDirG2QueryFindBase& theMsgR)
{
	SMsgDirG2QueryGetBase::operator=(theMsgR);
	mFindMode      = theMsgR.mFindMode;
	mFindFlags     = theMsgR.mFindFlags;
	mDisplayName   = theMsgR.mDisplayName;
	mSearchObjects = theMsgR.mSearchObjects;
	return *this;
}


// SMsgDirG2QueryFindBase::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2QueryFindBase::Pack(void)
{
	WTRACE("SMsgDirG2QueryFindBase::Pack");
	SMsgDirG2QueryGetBase::Pack();

	AppendByte(static_cast<unsigned char>(mFindMode));
	AppendByte(mFindFlags);
	return GetDataPtr();
}


// SMsgDirG2QueryFindBase::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2QueryFindBase::Unpack(void)
{
	WTRACE("SMsgDirG2QueryFindBase::Unpack");
	SMsgDirG2QueryGetBase::Unpack();
	mFindMode  = static_cast<DirG2FindMatchMode>(ReadByte());
	mFindFlags = ReadByte();
}
