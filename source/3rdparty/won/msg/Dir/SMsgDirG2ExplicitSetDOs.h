#ifndef _SMsgDirG2ExplicitSetDataObjects_H
#define _SMsgDirG2ExplicitSetDataObjects_H

// SMsgDirG2ExplicitSetDataObjects.h

// DirectoryServer explicit set data objects message.  Updates the set of data objects
// attached to an entity with a specified set of data objects.  Set mode is specified
// for each data object in the update set.

#include <string>
#include <list>
#include "common/DataObject.h"
#include "DirG2Flags.h"
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2ExplicitSetDataObjects : public SMsgDirG2UpdateBase
{
public:
	// Types
	struct ExplicitDOInfo
	{
		DirG2DataObjectSetMode mSetMode;  // Flags for set
		WONCommon::DataObject  mData;     // Data object to set

		ExplicitDOInfo() : mSetMode(DOSM_ADDREPLACE), mData() {}
	};
	typedef std::list<ExplicitDOInfo> ExplicitDOInfoList;

	// Default ctor
	explicit SMsgDirG2ExplicitSetDataObjects(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2ExplicitSetDataObjects(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2ExplicitSetDataObjects(const SMsgDirG2ExplicitSetDataObjects& theMsgR);

	// Destructor
	~SMsgDirG2ExplicitSetDataObjects(void);

	// Assignment
	SMsgDirG2ExplicitSetDataObjects& operator=(const SMsgDirG2ExplicitSetDataObjects& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Modify objects access
	const ExplicitDOInfoList& Objects() const;
	ExplicitDOInfoList&       Objects();

private:
	ExplicitDOInfoList mObjects;  // List of set objects
};


// Inlines
inline TRawMsg*
SMsgDirG2ExplicitSetDataObjects::Duplicate(void) const
{ return new SMsgDirG2ExplicitSetDataObjects(*this); }

inline const SMsgDirG2ExplicitSetDataObjects::ExplicitDOInfoList&
SMsgDirG2ExplicitSetDataObjects::Objects() const
{ return mObjects; }

inline SMsgDirG2ExplicitSetDataObjects::ExplicitDOInfoList&
SMsgDirG2ExplicitSetDataObjects::Objects()
{ return mObjects; }

};  // Namespace WONMsg

#endif