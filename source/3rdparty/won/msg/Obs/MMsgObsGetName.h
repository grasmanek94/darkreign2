#ifndef _MMsgObsGetName_H
#define _MMsgObsGetName_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetName
//
// The GetName message gets the unique name of the specified resource from the unique Identifier.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetName : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetName(void);

	// MiniMessage Constructor
	explicit MMsgObsGetName(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetName(const MMsgObsGetName& theMsgR);

	// Destructor
	~MMsgObsGetName(void);

	// Operator overloads

	// Assignment
	MMsgObsGetName& operator=(const MMsgObsGetName& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetName(*this);
	}

	// Member access methods
	unsigned long
	GetResourceId(void) const
	{
		return mResourceId;
	}

	ResourceTypeEnum
	GetResourceType(void) const
	{
		return mResourceType;
	}

	void
	SetResourceId(const unsigned long theResourceId)
	{
		mResourceId = theResourceId;
	}

	void
	SetResourceType(const ResourceTypeEnum theResourceType)
	{
		mResourceType = theResourceType;
	}
	
private:

	unsigned long		mResourceId;
	ResourceTypeEnum	mResourceType;
};

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNameReply
//
// The GetNameReply message is sent to the client or server that requested the name of the specified
// resource.  It contains status of the operation as well as the name of the resource specified by the
// ResourceId in the GetName message.  
//
///////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsGetNameReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsGetNameReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsGetNameReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsGetNameReply(const MMsgObsGetNameReply& theMsgR);

	~MMsgObsGetNameReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsGetNameReply& operator=(const MMsgObsGetNameReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsGetNameReply(*this);
	}

	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	const string&
	GetResourceName(void) const
	{ 
		return mResourceName;
	}

	const wstring&
	GetResourceDescription(void) const
	{ 
		return mResourceDescription;
	}

	void 
	SetStatus(const ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void 
	SetResourceName(const string& theResourceName)
	{ 
		mResourceName = theResourceName;
	}

	void 
	SetResourceDescription(const wstring& theResourceDescription)
	{ 
		mResourceDescription = theResourceDescription;
	}

private:
	ServerStatus	mStatus;				// Request status
	string			mResourceName;			// Name of publisher, publication, or subscriber
	wstring			mResourceDescription;	// Description of publisher, publication, or subscriber
};

}

#endif