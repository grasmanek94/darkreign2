#ifndef _SMsgDirG2SetDataObjects_H
#define _SMsgDirG2SetDataObjects_H

// SMsgDirG2SetDataObjects.h

// DirectoryServer set data objects message.  Updates the set of data
// objects attached to an entity.

#include <string>
#include "DirG2Flags.h"
#include "SMsgDirG2UpdateExtendBase.h"


namespace WONMsg {

class SMsgDirG2SetDataObjects : public SMsgDirG2UpdateExtendBase
{
public:
	// Default ctor
	explicit SMsgDirG2SetDataObjects(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2SetDataObjects(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2SetDataObjects(const SMsgDirG2SetDataObjects& theMsgR);

	// Destructor
	~SMsgDirG2SetDataObjects(void);

	// Assignment
	SMsgDirG2SetDataObjects& operator=(const SMsgDirG2SetDataObjects& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// EntityFlags access
	DirG2DataObjectSetMode GetSetMode() const;
	void SetSetMode(DirG2DataObjectSetMode theMode);

private:
	DirG2DataObjectSetMode mSetMode;  // Flags for set
};


// Inlines
inline TRawMsg*
SMsgDirG2SetDataObjects::Duplicate(void) const
{ return new SMsgDirG2SetDataObjects(*this); }

inline DirG2DataObjectSetMode
SMsgDirG2SetDataObjects::GetSetMode(void) const
{ return mSetMode; }

inline void
SMsgDirG2SetDataObjects::SetSetMode(DirG2DataObjectSetMode theMode)
{ mSetMode = theMode; }

};  // Namespace WONMsg

#endif