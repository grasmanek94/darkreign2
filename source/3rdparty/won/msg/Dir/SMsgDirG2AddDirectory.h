#ifndef _SMsgDirG2AddDirectory_H
#define _SMsgDirG2AddDirectory_H

// SMsgDirG2AddDirectory.h

// DirectoryServer add directory message.  Adds a new directory at specified path.

#include <string>
#include "SMsgDirG2UpdateExtendBase.h"


namespace WONMsg {

class SMsgDirG2AddDirectory : public SMsgDirG2UpdateExtendBase
{
public:
	// Default ctor
	explicit SMsgDirG2AddDirectory(bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2AddDirectory(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2AddDirectory(const SMsgDirG2AddDirectory& theMsgR);

	// Destructor
	~SMsgDirG2AddDirectory(void);

	// Assignment
	SMsgDirG2AddDirectory& operator=(const SMsgDirG2AddDirectory& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// EntityFlags access
	unsigned char GetEntityFlags() const;
	void SetEntityFlags(unsigned char theFlags);

	// Name access
	const std::wstring& GetDirName() const;
	void SetDirName(const std::wstring& theName);

	// DisplayName access
	const std::wstring& GetDisplayName() const;
	void SetDisplayName(const std::wstring& theName);

	// Lifespan access
	unsigned long GetLifespan() const;
	void SetLifespan(unsigned long theSpan);

private:
	unsigned char mEntityFlags;  // Flags for add
	std::wstring  mDirName;       // Directory name
	std::wstring  mDisplayName;  // Directory display name
	unsigned long mLifespan;     // Lifespan in seconds
};


// Inlines
inline TRawMsg*
SMsgDirG2AddDirectory::Duplicate(void) const
{ return new SMsgDirG2AddDirectory(*this); }

inline unsigned char
SMsgDirG2AddDirectory::GetEntityFlags(void) const
{ return mEntityFlags; }

inline void
SMsgDirG2AddDirectory::SetEntityFlags(unsigned char theFlags)
{ mEntityFlags = theFlags; }

inline const std::wstring&
SMsgDirG2AddDirectory::GetDirName(void) const
{ return mDirName; }

inline void
SMsgDirG2AddDirectory::SetDirName(const std::wstring& theName)
{ mDirName = theName; }

inline const std::wstring&
SMsgDirG2AddDirectory::GetDisplayName(void) const
{ return mDisplayName; }

inline void
SMsgDirG2AddDirectory::SetDisplayName(const std::wstring& theName)
{ mDisplayName = theName; }

inline unsigned long
SMsgDirG2AddDirectory::GetLifespan(void) const
{ return mLifespan; }

inline void
SMsgDirG2AddDirectory::SetLifespan(unsigned long theSpan)
{ mLifespan = theSpan; }

};  // Namespace WONMsg

#endif