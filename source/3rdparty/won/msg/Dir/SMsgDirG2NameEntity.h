#ifndef _SMsgDirG2NameEntity_H
#define _SMsgDirG2NameEntity_H

// SMsgDirG2NameEntity.h

// DirectoryServer name entity message.  Updates the display name for a specified
// entity.

#include <string>
#include "SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2NameEntity : public SMsgDirG2UpdateBase
{
public:
	// Default ctor
	explicit SMsgDirG2NameEntity(KeyType theType=KT_SERVICE);

	// SmallMessage ctor
	explicit SMsgDirG2NameEntity(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2NameEntity(const SMsgDirG2NameEntity& theMsgR);

	// Destructor
	~SMsgDirG2NameEntity(void);

	// Assignment
	SMsgDirG2NameEntity& operator=(const SMsgDirG2NameEntity& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// DisplayName access
	const std::wstring& GetDisplayName() const;
	void SetDisplayName(const std::wstring& theName);

	// Unique access
	bool IsUnique() const;
	void SetUnique(bool theFlag);

private:
	std::wstring mDisplayName;  // Directory display name
	bool         mUnique;       // Display name unique?
};


// Inlines
inline TRawMsg*
SMsgDirG2NameEntity::Duplicate(void) const
{ return new SMsgDirG2NameEntity(*this); }

inline const std::wstring&
SMsgDirG2NameEntity::GetDisplayName(void) const
{ return mDisplayName; }

inline void
SMsgDirG2NameEntity::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline bool
SMsgDirG2NameEntity::IsUnique(void) const
{ return mUnique; }

inline void
SMsgDirG2NameEntity::SetUnique(bool theFlag)
{ mUnique = theFlag; }

};  // Namespace WONMsg

#endif