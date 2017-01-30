#ifndef _SMsgDirG2RenewEntity_H
#define _SMsgDirG2RenewEntity_H

// SMsgDirG2RenewEntity.h

// DirectoryServer renew entity message.  Updates the lifespan for a specified
// entity.

#include <string>
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2RenewEntity : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2RenewEntity(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2RenewEntity(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2RenewEntity(const SMsgDirG2RenewEntity& theMsgR);

	// Destructor
	~SMsgDirG2RenewEntity(void);

	// Assignment
	SMsgDirG2RenewEntity& operator=(const SMsgDirG2RenewEntity& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Lifespan access
	unsigned long GetLifespan() const;
	void SetLifespan(unsigned long theSpan);

private:
	unsigned long mLifespan;  // Lifespan in seconds
};


// Inlines
inline TRawMsg*
SMsgDirG2RenewEntity::Duplicate(void) const
{ return new SMsgDirG2RenewEntity(*this); }

inline unsigned long
SMsgDirG2RenewEntity::GetLifespan(void) const
{ return mLifespan; }

inline void
SMsgDirG2RenewEntity::SetLifespan(unsigned long theSpan)
{ mLifespan = theSpan; }

};  // Namespace WONMsg

#endif