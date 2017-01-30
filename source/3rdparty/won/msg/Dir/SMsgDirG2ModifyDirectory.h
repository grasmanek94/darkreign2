#ifndef _SMsgDirG2ModifyDirectory_H
#define _SMsgDirG2ModifyDirectory_H

// SMsgDirG2ModifyDirectory.h

// DirectoryServer modify directory message.  Modifies attributes of a directory,
// including its name (key field).

#include <string>
#include "SMsgDirG2UpdateExtendBase.h"


namespace WONMsg {

class SMsgDirG2ModifyDirectory : public SMsgDirG2UpdateExtendBase
{
public:
	// Default ctor
	explicit SMsgDirG2ModifyDirectory(bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2ModifyDirectory(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2ModifyDirectory(const SMsgDirG2ModifyDirectory& theMsgR);

	// Destructor
	~SMsgDirG2ModifyDirectory(void);

	// Assignment
	SMsgDirG2ModifyDirectory& operator=(const SMsgDirG2ModifyDirectory& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// EntityFlags access
	unsigned char GetEntityFlags() const;
	void SetEntityFlags(unsigned char theFlags);

	// New Name access
	const std::wstring& GetNewName() const;
	void SetNewName(const std::wstring& theName);

	// New DisplayName access
	const std::wstring& GetNewDisplayName() const;
	void SetNewDisplayName(const std::wstring& theName);

	// New Lifespan access
	unsigned long GetNewLifespan() const;
	void SetNewLifespan(unsigned long theSpan);

private:
	unsigned char mEntityFlags;     // Flags for modify
	std::wstring  mNewName;         // New name
	std::wstring  mNewDisplayName;  // New display name
	unsigned long mNewLifespan;     // New lifespan
};


// Inlines
inline TRawMsg*
SMsgDirG2ModifyDirectory::Duplicate(void) const
{ return new SMsgDirG2ModifyDirectory(*this); }

inline unsigned char
SMsgDirG2ModifyDirectory::GetEntityFlags(void) const
{ return mEntityFlags; }

inline void
SMsgDirG2ModifyDirectory::SetEntityFlags(unsigned char theFlags)
{ mEntityFlags = theFlags; }

inline const std::wstring&
SMsgDirG2ModifyDirectory::GetNewName(void) const
{ return mNewName; }

inline void
SMsgDirG2ModifyDirectory::SetNewName(const std::wstring& theName)
{ mNewName = theName; }

inline const std::wstring&
SMsgDirG2ModifyDirectory::GetNewDisplayName(void) const
{ return mNewDisplayName; }

inline void
SMsgDirG2ModifyDirectory::SetNewDisplayName(const std::wstring& theName)
{ mNewDisplayName = theName; }

inline unsigned long
SMsgDirG2ModifyDirectory::GetNewLifespan(void) const
{ return mNewLifespan; }

inline void
SMsgDirG2ModifyDirectory::SetNewLifespan(unsigned long theSpan)
{ mNewLifespan = theSpan; }

};  // Namespace WONMsg

#endif