#ifndef _SMsgDirG2FindDirectory_H
#define _SMsgDirG2FindDirectory_H

// SMsgDirG2FindDirectory.h

// DirectoryServer find directory message.  Locates one or more directories based on
// key fields.  Fields of directories(s) and their contents included in reply are
// configurable.

#include "SMsgDirG2QueryFindBase.h"


namespace WONMsg {

class SMsgDirG2FindDirectory : public SMsgDirG2QueryFindBase
{
public:
	// Default ctor
	explicit SMsgDirG2FindDirectory(bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2FindDirectory(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2FindDirectory(const SMsgDirG2FindDirectory& theMsgR);

	// Destructor
	~SMsgDirG2FindDirectory(void);

	// Assignment
	SMsgDirG2FindDirectory& operator=(const SMsgDirG2FindDirectory& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Dir Name access
	const std::wstring& GetDirName() const;
	void SetDirName(const std::wstring& theName);

private:
	std::wstring mDirName;  // Dir name search key
};


// Inlines
inline TRawMsg*
SMsgDirG2FindDirectory::Duplicate(void) const
{ return new SMsgDirG2FindDirectory(*this); }

inline const std::wstring&
SMsgDirG2FindDirectory::GetDirName(void) const
{ return mDirName; }

inline void
SMsgDirG2FindDirectory::SetDirName(const std::wstring& theName)
{ mDirName = theName; }

};  // Namespace WONMsg

#endif