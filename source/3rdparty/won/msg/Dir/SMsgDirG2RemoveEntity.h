#ifndef _SMsgDirG2RemoveEntity_H
#define _SMsgDirG2RemoveEntity_H

// SMsgDirG2RemoveEntity.h

// DirectoryServer remove entity message.  Removes a specified entity.

#include <string>
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2RemoveEntity : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2RemoveEntity(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2RemoveEntity(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2RemoveEntity(const SMsgDirG2RemoveEntity& theMsgR);

	// Destructor
	~SMsgDirG2RemoveEntity(void);

	// Assignment
	SMsgDirG2RemoveEntity& operator=(const SMsgDirG2RemoveEntity& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

private:
};


// Inlines
inline TRawMsg*
SMsgDirG2RemoveEntity::Duplicate(void) const
{ return new SMsgDirG2RemoveEntity(*this); }

};  // Namespace WONMsg

#endif