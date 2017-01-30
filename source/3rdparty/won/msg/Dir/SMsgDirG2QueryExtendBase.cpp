// SMsgDirG2QueryExtendBase.h

// Base class derived from SMsgDirG2QueryBase that provides common definitions for
// extended queries. Adds list of data object get types.  Also logical extended flag
// (is query an extended query).

// Derived class should set the mExtended member as needed to specify whether query
// is extended or not.  This should be done in constructors and Unpack.  Note that
// mExtended defaults to false.

// Adds a PackExtended() and UnpackExtended() methods to pack/unpack the data object
// get types.  These methods should be called be derived class Pack/Unpack at the
// appropriate point to pack/unpack extended data if needed.  Note that these methods
// are NoOps if message is not extended (mExtended == false).


#include "common/won.h"
#include "msg/TMessage.h"
#include "DirEntity.h"
#include "SMsgDirG2QueryExtendBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryBase;
	using WONMsg::SMsgDirG2QueryExtendBase;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2QueryExtendBase::SMsgDirG2QueryExtendBase(KeyType theType, bool isExtended) :
	SMsgDirG2QueryBase(theType),
	mExtended(isExtended),
	mGetTypes()
{}


// SmallMessage ctor
SMsgDirG2QueryExtendBase::SMsgDirG2QueryExtendBase(const SmallMessage& theMsgR) :
	SMsgDirG2QueryBase(theMsgR),
	mExtended(false),
	mGetTypes()
{}


// Copy ctor
SMsgDirG2QueryExtendBase::SMsgDirG2QueryExtendBase(const SMsgDirG2QueryExtendBase& theMsgR) :
	SMsgDirG2QueryBase(theMsgR),
	mExtended(theMsgR.mExtended),
	mGetTypes(theMsgR.mGetTypes)
{}


// Destructor
SMsgDirG2QueryExtendBase::~SMsgDirG2QueryExtendBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2QueryExtendBase::PackExtended(void)
{
	WTRACE("SMsgDirG2QueryExtendBase::PackExtended");
	WDBG_LL("SMsgDirG2QueryExtendBase::PackExtended Extended=" << mExtended << " GetTypes size=" << mGetTypes.size());
	if (! mExtended) return;

	AppendShort(mGetTypes.size());
	DataObjectTypeSet::iterator anItr(mGetTypes.begin());
	for (; anItr != mGetTypes.end(); anItr++)
	{
		unsigned char aLen = anItr->GetDataType().size();
		AppendByte(aLen);
		if (aLen > 0)
			AppendBytes(aLen, anItr->GetDataType().data());
	}
}


void
SMsgDirG2QueryExtendBase::UnpackExtended(void)
{
	WTRACE("SMsgDirG2QueryExtendBase::UnpackExtended");
	WDBG_LL("SMsgDirG2QueryExtendBase::UnpackExtended Extended=" << mExtended);
	mGetTypes.clear();
	if (! mExtended) return;

	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2QueryExtendBase::UnpackExtended Reading " << aCt << " entries.");

	for (int i=0; i < aCt; i++)
	{
		DataObject anObj;
		unsigned char aLen = ReadByte();
		if (aLen > 0)
		{
			anObj.GetDataType().assign(reinterpret_cast<const unsigned char*>(ReadBytes(aLen)), aLen);
			mGetTypes.insert(anObj);
		}
	}
}


// ** Public Methods **

// Assignment operator
SMsgDirG2QueryExtendBase&
SMsgDirG2QueryExtendBase::operator=(const SMsgDirG2QueryExtendBase& theMsgR)
{
	SMsgDirG2QueryBase::operator=(theMsgR);
	mExtended  = theMsgR.mExtended;
	mGetTypes = theMsgR.mGetTypes;
	return *this;
}
