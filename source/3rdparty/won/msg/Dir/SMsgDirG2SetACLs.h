#ifndef _SMsgDirG2SetACLs_H
#define _SMsgDirG2SetACLs_H

// SMsgDirG2SetACLs.h

// DirectoryServer set ACLs message.  Updates the ACLs attached to an entity.
// This message can be used to any combinations of add, modify, or remove
// of permissions within all types of ACLs on an entity.

#include <string>
#include "DirG2Flags.h"
#include "DirACLs.h"
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2SetACLs : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2SetACLs(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2SetACLs(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2SetACLs(const SMsgDirG2SetACLs& theMsgR);

	// Destructor
	~SMsgDirG2SetACLs(void);

	// Assignment
	SMsgDirG2SetACLs& operator=(const SMsgDirG2SetACLs& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// ACL access
	const DirUpdateACLList& GetACLs() const;
	DirUpdateACLList& GetACLs();
	void SetACLs(const DirUpdateACLList& theSetR);
	void AddACL(const DirUpdateACL& theObjR);

private:
	DirUpdateACLList mACLs;  // List of update ACLs
};


// Inlines
inline TRawMsg*
SMsgDirG2SetACLs::Duplicate(void) const
{ return new SMsgDirG2SetACLs(*this); }

inline const DirUpdateACLList&
SMsgDirG2SetACLs::GetACLs() const
{ return mACLs; }

inline DirUpdateACLList&
SMsgDirG2SetACLs::GetACLs()
{ return mACLs; }

inline void
SMsgDirG2SetACLs::SetACLs(const DirUpdateACLList& theSetR)
{ mACLs = theSetR; }

inline void
SMsgDirG2SetACLs::AddACL(const DirUpdateACL& theObjR)
{ mACLs.push_back(theObjR); }

};  // Namespace WONMsg

#endif