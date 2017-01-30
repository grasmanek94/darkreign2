#ifndef _SMsgDirG2ModifyService_H
#define _SMsgDirG2ModifyService_H

// SMsgDirG2ModifyService.h

// DirectoryServer modify service message.  Modifies attributes of a service,
// including its name or net address (key fields).

#include <string>
#include "common/won.h"
#include "SMsgDirG2UpdateExtendBase.h"


namespace WONMsg {

class SMsgDirG2ModifyService : public SMsgDirG2UpdateExtendBase
{
public:
	// Default ctor
	explicit SMsgDirG2ModifyService(bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2ModifyService(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2ModifyService(const SMsgDirG2ModifyService& theMsgR);

	// Destructor
	~SMsgDirG2ModifyService(void);

	// Assignment
	SMsgDirG2ModifyService& operator=(const SMsgDirG2ModifyService& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// EntityFlags access
	unsigned char GetEntityFlags() const;
	void SetEntityFlags(unsigned char theFlags);

	// New name access
	const std::wstring& GetNewName() const;
	void SetNewName(const std::wstring& theName);

	// New netAddress access
	const WONCommon::RawBuffer& GetNewNetAddress() const;
	void SetNewNetAddress(const WONCommon::RawBuffer& theAddr);

	// DisplayName access
	const std::wstring& GetNewDisplayName() const;
	void SetNewDisplayName(const std::wstring& theName);

	// Lifespan access
	unsigned long GetNewLifespan() const;
	void SetNewLifespan(unsigned long theSpan);

private:
	unsigned char        mEntityFlags;     // Flags for modify
	std::wstring         mNewName;         // New name
	WONCommon::RawBuffer mNewNetAddress;   // New net address
	std::wstring         mNewDisplayName;  // New display name
	unsigned long        mNewLifespan;     // New lifespan in seconds
};


// Inlines
inline TRawMsg*
SMsgDirG2ModifyService::Duplicate(void) const
{ return new SMsgDirG2ModifyService(*this); }

inline unsigned char
SMsgDirG2ModifyService::GetEntityFlags(void) const
{ return mEntityFlags; }

inline void
SMsgDirG2ModifyService::SetEntityFlags(unsigned char theFlags)
{ mEntityFlags = theFlags; }

inline const std::wstring&
SMsgDirG2ModifyService::GetNewName() const
{ return mNewName; }

inline void
SMsgDirG2ModifyService::SetNewName(const std::wstring& theName)
{ mNewName = theName; }

inline const WONCommon::RawBuffer&
SMsgDirG2ModifyService::GetNewNetAddress() const
{ return mNewNetAddress; }

inline void
SMsgDirG2ModifyService::SetNewNetAddress(const WONCommon::RawBuffer& theAddr)
{ mNewNetAddress = theAddr; }

inline const std::wstring&
SMsgDirG2ModifyService::GetNewDisplayName(void) const
{ return mNewDisplayName; }

inline void
SMsgDirG2ModifyService::SetNewDisplayName(const std::wstring& theName)
{ mNewDisplayName = theName; }

inline unsigned long
SMsgDirG2ModifyService::GetNewLifespan(void) const
{ return mNewLifespan; }

inline void
SMsgDirG2ModifyService::SetNewLifespan(unsigned long theSpan)
{ mNewLifespan = theSpan; }

};  // Namespace WONMsg

#endif