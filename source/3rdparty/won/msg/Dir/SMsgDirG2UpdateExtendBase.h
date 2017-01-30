#ifndef _SMsgDirG2UpdateExtendBase_H
#define _SMsgDirG2UpdateExtendBase_H

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


#include <string>
#include "common/DataObject.h"
#include "DirACLs.h"
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2UpdateExtendBase : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2UpdateExtendBase(KeyType theType, bool enableDataObjects,
	                                   bool enableACLs);

	// SmallMessage ctor
	explicit SMsgDirG2UpdateExtendBase(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2UpdateExtendBase(const SMsgDirG2UpdateExtendBase& theMsgR);

	// Destructor
	virtual ~SMsgDirG2UpdateExtendBase(void);

	// Assignment
	SMsgDirG2UpdateExtendBase& operator=(const SMsgDirG2UpdateExtendBase& theMsgR);

	// Virtual Duplicate from SmallMessage
	// Pure virtual - must be overridden!
	virtual TRawMsg* Duplicate(void) const = 0;

	// Extended info enabler access
	bool IsExtended() const;
	void SetExtended(bool enableDataObjects, bool enableACLs);
	bool IsEnableDataObjects() const;
	bool IsEnableACLs() const;

	// Data Object access
	const WONCommon::DataObjectTypeSet& GetDataObjects() const;
	WONCommon::DataObjectTypeSet& GetDataObjects();
	void SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR);
	void AddDataObject(const WONCommon::DataObject& theObjR);

	// ACL access
	const DirACLList& GetACLs() const;
	DirACLList& GetACLs();
	void SetACLs(const DirACLList& theSetR);
	void AddACL(const DirACL& theObjR);

protected:
	WONCommon::DataObjectTypeSet mDataObjects;        // Set of data objects for extended updates
	DirACLList                   mACLs;               // List of ACLs
	bool                         mEnableDataObjects;  // Enable data objects in pack/unpack
	bool                         mEnableACLs;         // Enable ACLs in pack and unpack

	// Pack extended info into raw buffer (call in Pack()).
	virtual void PackExtended();

	// Unpack extended info from raw buffer (call in Unpack()).
	virtual void UnpackExtended();

private:
};


// Inlines
inline bool
SMsgDirG2UpdateExtendBase::IsExtended() const
{ return ((mEnableDataObjects) || (mEnableACLs)); }

inline void
SMsgDirG2UpdateExtendBase::SetExtended(bool enableDataObjects, bool enableACLs)
{ mEnableDataObjects = enableDataObjects;  mEnableACLs = enableACLs; }

inline bool
SMsgDirG2UpdateExtendBase::IsEnableDataObjects() const
{ return mEnableDataObjects; }

inline bool
SMsgDirG2UpdateExtendBase::IsEnableACLs() const
{ return mEnableACLs; }

inline const WONCommon::DataObjectTypeSet&
SMsgDirG2UpdateExtendBase::GetDataObjects() const
{ return mDataObjects; }

inline WONCommon::DataObjectTypeSet&
SMsgDirG2UpdateExtendBase::GetDataObjects()
{ return mDataObjects; }

inline void
SMsgDirG2UpdateExtendBase::SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR)
{ mDataObjects = theSetR; }

inline void
SMsgDirG2UpdateExtendBase::AddDataObject(const WONCommon::DataObject& theObjR)
{ mDataObjects.insert(theObjR); }

inline const DirACLList&
SMsgDirG2UpdateExtendBase::GetACLs() const
{ return mACLs; }

inline DirACLList&
SMsgDirG2UpdateExtendBase::GetACLs()
{ return mACLs; }

inline void
SMsgDirG2UpdateExtendBase::SetACLs(const DirACLList& theSetR)
{ mACLs = theSetR; }

inline void
SMsgDirG2UpdateExtendBase::AddACL(const DirACL& theObjR)
{ mACLs.push_back(theObjR); }


};  // Namespace WONMsg

#endif