#ifndef _SMsgDirG2ClearDataObjects_H
#define _SMsgDirG2ClearDataObjects_H

// SMsgDirG2ClearDataObjects.h

// DirectoryServer clear data objects message.  Removes specified data
// objects from the set of data objects attached to an entity.

#include <string>
#include "common/DataObject.h"
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2ClearDataObjects : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2ClearDataObjects(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2ClearDataObjects(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2ClearDataObjects(const SMsgDirG2ClearDataObjects& theMsgR);

	// Destructor
	~SMsgDirG2ClearDataObjects(void);

	// Assignment
	SMsgDirG2ClearDataObjects& operator=(const SMsgDirG2ClearDataObjects& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Get Data Types access
	const WONCommon::DataObjectTypeSet& GetClearTypes() const;
	void SetClearTypes(const WONCommon::DataObjectTypeSet& theSetR);
	void AddClearType(const WONCommon::DataObject& theTypeR);
	void AddClearType(const WONCommon::DataObject::DataType& theTypeR);
	void AddClearType(const unsigned char* theTypeP, unsigned char theLen);

private:
	WONCommon::DataObjectTypeSet mClearTypes;  // Set of data object get types for extended query
};


// Inlines
inline TRawMsg*
SMsgDirG2ClearDataObjects::Duplicate(void) const
{ return new SMsgDirG2ClearDataObjects(*this); }

inline const WONCommon::DataObjectTypeSet&
SMsgDirG2ClearDataObjects::GetClearTypes() const
{ return mClearTypes; }

inline void
SMsgDirG2ClearDataObjects::SetClearTypes(const WONCommon::DataObjectTypeSet& theSetR)
{ mClearTypes = theSetR; }

inline void
SMsgDirG2ClearDataObjects::AddClearType(const WONCommon::DataObject& theTypeR)
{ mClearTypes.insert(theTypeR); }

inline void
SMsgDirG2ClearDataObjects::AddClearType(const WONCommon::DataObject::DataType& theTypeR)
{ mClearTypes.insert(WONCommon::DataObject(theTypeR)); }

inline void
SMsgDirG2ClearDataObjects::AddClearType(const unsigned char* theTypeP, unsigned char theLen)
{ mClearTypes.insert(WONCommon::DataObject(WONCommon::DataObject::DataType(theTypeP, theLen))); }

};  // Namespace WONMsg

#endif