// SMsgDirG2QueryGetBase.h

// Base class derived from SMsgDirG2QueryExtendBase that provides common definitions
// for get queries. Adds fields for GetFlags.

// Derived class should set the mExtended member from SMsgDirG2QueryExtendBase as
// needed to specify whether query is extended or not.  This should be done in
// constructors and Unpack.  Note that mExtended defaults to false.

// Note that the Pack and Unpack() methods append/write the getflags.  This implies
// that all classes derived from QueryGetBase MUST have getflags as the first field
// after their message header.

// Note: SMsgDirg2QueryGetbase does not override the pure virtual Duplicate() method
// from SMsgDirG2QueryExtendBase.  Derrived classes must override this method.



#include "common/won.h"
#include "msg/TMessage.h"
#include "DirEntity.h"
#include "SMsgDirG2QueryGetBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryExtendBase;
	using WONMsg::SMsgDirG2QueryGetBase;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2QueryGetBase::SMsgDirG2QueryGetBase(KeyType theType, bool isExtended) :
	SMsgDirG2QueryExtendBase(theType, isExtended),
	mFlags(WONMsg::GF_NOFLAGS),
	mEntitiesPerReply(0)
{}


// SmallMessage ctor
SMsgDirG2QueryGetBase::SMsgDirG2QueryGetBase(const SmallMessage& theMsgR) :
	SMsgDirG2QueryExtendBase(theMsgR),
	mFlags(WONMsg::GF_NOFLAGS),
	mEntitiesPerReply(0)
{}


// Copy ctor
SMsgDirG2QueryGetBase::SMsgDirG2QueryGetBase(const SMsgDirG2QueryGetBase& theMsgR) :
	SMsgDirG2QueryExtendBase(theMsgR),
	mFlags(theMsgR.mFlags),
	mEntitiesPerReply(theMsgR.mEntitiesPerReply)
{}


// Destructor
SMsgDirG2QueryGetBase::~SMsgDirG2QueryGetBase(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2QueryGetBase&
SMsgDirG2QueryGetBase::operator=(const SMsgDirG2QueryGetBase& theMsgR)
{
	SMsgDirG2QueryExtendBase::operator=(theMsgR);
	mFlags            = theMsgR.mFlags;
	mEntitiesPerReply = theMsgR.mEntitiesPerReply;
	return *this;
}


// SMsgDirG2QueryGetBase::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.  Note that
// mEntriesPerReply IS NOT affected.
void*
SMsgDirG2QueryGetBase::Pack(void)
{
	WTRACE("SMsgDirG2QueryGetBase::Pack");
	SMsgDirG2QueryExtendBase::Pack();

	// Add GetType to flags if any entity specific data requested
	unsigned long aTst = (WONMsg::GF_DIRADDPATH    | WONMsg::GF_DIRADDNAME  |
	                      WONMsg::GF_DIRADDVISIBLE | WONMsg::GF_SERVADDPATH |
	                      WONMsg::GF_SERVADDNAME   | WONMsg::GF_SERVADDNETADDR);
	if (mFlags & aTst)
		mFlags |= WONMsg::GF_ADDTYPE;
		
	AppendLong(mFlags);
	return GetDataPtr();
}


// SMsgDirG2QueryGetBase::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.  Note that
// mEntriesPerReply IS NOT affected.
void
SMsgDirG2QueryGetBase::Unpack(void)
{
	WTRACE("SMsgDirG2QueryGetBase::Unpack");
	SMsgDirG2QueryExtendBase::Unpack();

	// Read flags, force AddDataObjects and AddPermissions to off if not extended
	mFlags = ReadLong();
	if (! mExtended)
		mFlags &= (WONMsg::GF_ALLFLAGS ^ (WONMsg::GF_ADDDATAOBJECTS | WONMsg::GF_ADDACLS));
}
