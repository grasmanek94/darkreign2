#ifndef _SMsgDirG2FindService_H
#define _SMsgDirG2FindService_H

// SMsgDirG2FindService.h

// DirectoryServer find service message.  Locates one or more services based on
// key fields.  Fields of service(s) included in reply are configurable.

#include "SMsgDirG2QueryFindBase.h"


namespace WONMsg {

class SMsgDirG2FindService : public SMsgDirG2QueryFindBase
{
public:
	// Default ctor
	explicit SMsgDirG2FindService(bool isExtended=false);

	// SmallMessage ctor
	explicit SMsgDirG2FindService(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2FindService(const SMsgDirG2FindService& theMsgR);

	// Destructor
	~SMsgDirG2FindService(void);

	// Assignment
	SMsgDirG2FindService& operator=(const SMsgDirG2FindService& theMsgR);

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
SMsgDirG2FindService::Duplicate(void) const
{ return new SMsgDirG2FindService(*this); }

};  // Namespace WONMsg

#endif