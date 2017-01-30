#ifndef _SMsgDirG2QueryFindBase_H
#define _SMsgDirG2QueryFindBase_H

// SMsgDirG2QueryFindBase.h

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


#include <string>
#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "DirG2Flags.h"
#include "SMsgDirG2QueryGetBase.h"


namespace WONMsg {

class SMsgDirG2QueryFindBase : public SMsgDirG2QueryGetBase
{
public:
	// Default ctor
	explicit SMsgDirG2QueryFindBase(KeyType theType, bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2QueryFindBase(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2QueryFindBase(const SMsgDirG2QueryFindBase& theMsgR);

	// Destructor
	virtual ~SMsgDirG2QueryFindBase(void);

	// Assignment
	SMsgDirG2QueryFindBase& operator=(const SMsgDirG2QueryFindBase& theMsgR);

	// Pack and Unpack the message
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// FindMode access
	DirG2FindMatchMode GetFindMode() const;
	void               SetFindMode(DirG2FindMatchMode theMode);
	
	// FindFlags access
	unsigned char GetFindFlags() const;
	void          SetFindFlags(unsigned char theFlags);
	
	// DisplayName access
	const std::wstring& GetDisplayName() const;
	void SetDisplayName(const std::wstring& theName);

	// Data Object access
	const WONCommon::DataObjectTypeSet& GetSearchObjects() const;
	void SetSearchObjects(const WONCommon::DataObjectTypeSet& theSetR);
	void AddSearchObject(const WONCommon::DataObject& theObjR);

protected:
	DirG2FindMatchMode           mFindMode;       // Find Mode
	unsigned char                mFindFlags;      // Find Flags
	std::wstring                 mDisplayName;    // Display name search key
	WONCommon::DataObjectTypeSet mSearchObjects;  // Data object serach key(s)

	// Pack search objects - call in dervied Pack()
	void PackSearchObjects();

	// Unpack search objects - call inb dervied Unpack()
	void UnpackSearchObjects();

private:
};


// Inlines
inline DirG2FindMatchMode
SMsgDirG2QueryFindBase::GetFindMode() const
{ return mFindMode; }

inline void
SMsgDirG2QueryFindBase::SetFindMode(DirG2FindMatchMode theMode)
{ mFindMode = theMode; }

inline unsigned char
SMsgDirG2QueryFindBase::GetFindFlags() const
{ return mFindFlags; }

inline void
SMsgDirG2QueryFindBase::SetFindFlags(unsigned char theFlags)
{ mFindFlags = theFlags; }

inline const std::wstring&
SMsgDirG2QueryFindBase::GetDisplayName(void) const
{ return mDisplayName; }

inline void
SMsgDirG2QueryFindBase::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline const WONCommon::DataObjectTypeSet&
SMsgDirG2QueryFindBase::GetSearchObjects() const
{ return mSearchObjects; }

inline void
SMsgDirG2QueryFindBase::SetSearchObjects(const WONCommon::DataObjectTypeSet& theSetR)
{ mSearchObjects = theSetR; }

inline void
SMsgDirG2QueryFindBase::AddSearchObject(const WONCommon::DataObject& theObjR)
{ mSearchObjects.insert(theObjR); }

};  // Namespace WONMsg

#endif