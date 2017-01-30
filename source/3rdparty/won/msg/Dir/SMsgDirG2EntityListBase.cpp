// SMsgDirG2EntityListBase

// Base class derived from SMsgDirG2EntityBase that adds a list of DirEntity
// objects and a sequence/last reply byte.  Does not override Pack/Unpack.

// Adds a PackEntities() and UnpackEntities() methods to pack/unpack the list of
// entities.  These methods should be called be derived class Pack/Unpack.

// Adds a PackSequence() and UnpackSequence() methods tp pack/unpack the sequence
// and last reply flag.  These methods should be called be derived class Pack/Unpack.

// Adds ComputePackSize().  This method returns the size in bytes required to
// pack the current list of DirEntity objects and the sequence byte.

// Note: This class does not override the pure virtual Duplicate() and GetFlags()
// methods from SMsgDirG2EntityBase.  Derived classes must still override
// these methods.


#include "common/won.h"
#include "msg/TMessage.h"
#include "DirEntity.h"
#include "SMsgDirG2EntityListBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::DirEntity;
	using WONMsg::DirEntityList;
	using WONMsg::SMsgDirG2EntityListBase;

	// Constants
	const unsigned char SET_LASTREPLY_BIT   = 0x80;
	const unsigned char CLEAR_LASTREPLY_BIT = 0x7f;
};


// ** SMsgDirG2EntityListBase **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2EntityListBase::SMsgDirG2EntityListBase(void) :
	SMsgDirG2EntityBase(),
	mEntities(),
	mSequence(0),
	mLastReply(false)
{}


// SmallMessage ctor
SMsgDirG2EntityListBase::SMsgDirG2EntityListBase(const SmallMessage& theMsgR) :
	SMsgDirG2EntityBase(theMsgR),
	mEntities(),
	mSequence(0),
	mLastReply(false)
{}


// Copy ctor
SMsgDirG2EntityListBase::SMsgDirG2EntityListBase(const SMsgDirG2EntityListBase& theMsgR) :
	SMsgDirG2EntityBase(theMsgR),
	mEntities(theMsgR.mEntities),
	mSequence(theMsgR.mSequence),
	mLastReply(theMsgR.mLastReply)
{}


// Destructor
SMsgDirG2EntityListBase::~SMsgDirG2EntityListBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2EntityListBase::PackEntities(void)
{
	WTRACE("SMsgDirG2EntityListBase::PackEntities");
	WDBG_LL("SMsgDirG2EntityListBase::PackEntities Appending " << mEntities.size() << "entries.");
	AppendShort(mEntities.size());
	DirEntityList::iterator anItr(mEntities.begin());
	for (; anItr != mEntities.end(); anItr++)
		PackEntity(*anItr);
}


void
SMsgDirG2EntityListBase::UnpackEntities(void)
{
	WTRACE("SMsgDirG2EntityListBase::UnpackEntities");
	unsigned short anEntityCt = ReadShort();
	WDBG_LL("SMsgDirG2EntityListBase::UnpackEntities Reading " << anEntityCt << "entries.");

	mEntities.clear();
	for (int i=0; i < anEntityCt; i++)
	{
		DirEntity anEntity;
		mEntities.push_back(anEntity);
		UnpackEntity(mEntities.back());
	}
}


void
SMsgDirG2EntityListBase::PackSequence(void)
{
	WTRACE("SMsgDirG2EntityListBase::PackSequence");
	WDBG_LL("SMsgDirG2EntityListBase::PackSequence Sequence=" << mSequence << " lastReply=" << mLastReply);

	unsigned char aPack = mSequence;
	if (mLastReply)
		aPack |= SET_LASTREPLY_BIT;

	AppendByte(aPack);
}


void
SMsgDirG2EntityListBase::UnpackSequence(void)
{
	WTRACE("SMsgDirG2EntityListBase::UnpackSequence");
	mSequence = ReadByte();
	WDBG_LL("SMsgDirG2EntityListBase::PackSequence Read sequence=" << mSequence);

	mLastReply = ((mSequence & SET_LASTREPLY_BIT) != 0);
	if (mLastReply)
		mSequence &= CLEAR_LASTREPLY_BIT;
}


// ** Public Methods **

// Assignment operator
SMsgDirG2EntityListBase&
SMsgDirG2EntityListBase::operator=(const SMsgDirG2EntityListBase& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SMsgDirG2EntityBase::operator=(theMsgR);
		mEntities  = theMsgR.mEntities;
		mSequence  = theMsgR.mSequence;
		mLastReply = theMsgR.mLastReply;
	}
	return *this;
}


unsigned long
SMsgDirG2EntityListBase::ComputePackSize() const
{
	unsigned long aRet = GetHeaderLength() + sizeof(mSequence) + sizeof(unsigned short);

	DirEntityList::const_iterator anItr(mEntities.begin());
	for (; anItr != mEntities.end(); anItr++)
		aRet += anItr->ComputeSize(GetFlags(), mDataTypes);

	return aRet;
}
