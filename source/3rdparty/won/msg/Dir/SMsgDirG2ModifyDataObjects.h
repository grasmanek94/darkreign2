#ifndef _SMsgDirG2ModifyDataObjects_H
#define _SMsgDirG2ModifyDataObjects_H

// SMsgDirG2ModifyDataObjects.h

// DirectoryServer modify data objects message.  Insert data or replace all or
// part of the data in the data field of specified data objects for specified entity.

#include <string>
#include <list>
#include "common/DataObject.h"
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2ModifyDataObjects : public SMsgDirG2UpdateBase
{
public:
	// Types
	struct ModifyDOInfo
	{
		unsigned short        mOffset;    // Starting offset for modify
		bool                  mIsInsert;  // Insert or replace data?
		WONCommon::DataObject mData;      // Type of data object to modify & modify data

		ModifyDOInfo() : mOffset(0), mIsInsert(false), mData() {}
	};
	typedef std::list<ModifyDOInfo> ModifyDOInfoList;

	// Default ctor
	explicit SMsgDirG2ModifyDataObjects(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2ModifyDataObjects(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2ModifyDataObjects(const SMsgDirG2ModifyDataObjects& theMsgR);

	// Destructor
	~SMsgDirG2ModifyDataObjects(void);

	// Assignment
	SMsgDirG2ModifyDataObjects& operator=(const SMsgDirG2ModifyDataObjects& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Modify objects access
	const ModifyDOInfoList& Objects() const;
	ModifyDOInfoList&       Objects();

private:
	ModifyDOInfoList mObjects;  // List of modify objects
};


// Inlines
inline TRawMsg*
SMsgDirG2ModifyDataObjects::Duplicate(void) const
{ return new SMsgDirG2ModifyDataObjects(*this); }

inline const SMsgDirG2ModifyDataObjects::ModifyDOInfoList&
SMsgDirG2ModifyDataObjects::Objects() const
{ return mObjects; }

inline SMsgDirG2ModifyDataObjects::ModifyDOInfoList&
SMsgDirG2ModifyDataObjects::Objects()
{ return mObjects; }

};  // Namespace WONMsg

#endif