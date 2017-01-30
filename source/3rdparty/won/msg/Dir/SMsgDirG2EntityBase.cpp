// SMsgDirG2EntityBase

// Base class derived from SmallMessage that adds the capability to pack and unpack
// DirEntities.  Adds a container of data object types that may be used to specify
// which data object of the entity to pack.
// Does not override Pack/Unpack.

// Adds pure virtual GetFlags() method.  This method must be overridden to return
// the set of get flags for packing the entity.

// Adds a PackEntity() and UnpackEntity() methods to pack/unpack an entity.
// These methods should be called by the derived class Pack/Unpack.


#include "common/won.h"
#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "DirEntity.h"
#include "SMsgDirG2EntityBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectTypeSet;
	using WONMsg::SmallMessage;
	using WONMsg::DirEntity;
	using WONMsg::SMsgDirG2EntityBase;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2EntityBase::SMsgDirG2EntityBase(void) :
	SmallMessage(),
	mDataTypes()
{}


// SmallMessage ctor
SMsgDirG2EntityBase::SMsgDirG2EntityBase(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mDataTypes()
{}


// Copy ctor
SMsgDirG2EntityBase::SMsgDirG2EntityBase(const SMsgDirG2EntityBase& theMsgR) :
	SmallMessage(theMsgR),
	mDataTypes(theMsgR.mDataTypes)
{}


// Destructor
SMsgDirG2EntityBase::~SMsgDirG2EntityBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2EntityBase::PackEntity(const DirEntity& theEntity)
{
	WTRACE("SMsgDirG2EntityBase::PackEntity");
	WDBG_LL("SMsgDirG2EntityBase::PackEntity Packing entity, type=" << theEntity.mType);
	theEntity.Pack(*this, GetFlags(), mDataTypes);
}


void
SMsgDirG2EntityBase::UnpackEntity(DirEntity& theEntity)
{
	WTRACE("SMsgDirG2EntityBase::UnpackEntity");
	WDBG_LL("SMsgDirG2EntityBase::UnpackEntity Unpacking entity");
	theEntity.Unpack(*this, GetFlags());
}


// ** Public Methods **

// Assignment operator
SMsgDirG2EntityBase&
SMsgDirG2EntityBase::operator=(const SMsgDirG2EntityBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SmallMessage::operator=(theMsgR);
		mDataTypes = theMsgR.mDataTypes;
	}
	return *this;
}
