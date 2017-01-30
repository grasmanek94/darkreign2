#ifndef _MMsgObsEnumeratePublications_H
#define _MMsgObsEnumeratePublications_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumeratePublications
//
// The EnumeratePublications requests a list of the available Publications on the Observation
// server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsEnumeratePublications : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumeratePublications(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumeratePublications(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumeratePublications(const MMsgObsEnumeratePublications& theMsgR);

	// Destructor
	~MMsgObsEnumeratePublications(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumeratePublications& operator=(const MMsgObsEnumeratePublications& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumeratePublications(*this);
	}

	// Member access methods
	unsigned long
	GetPublisherId(void) const
	{
		return mPublisherId;
	}

	void
	SetPublisherId(const unsigned long thePublisherId)
	{
		mPublisherId = thePublisherId;
	}

private:

	unsigned long	mPublisherId;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// MMsgObsEnumeratePublicationsReply
//
// The EnumeratePublicationReply message contains a list of all of the publications on the
// Observation server published by the specified publisher.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsEnumeratePublicationsReply : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumeratePublicationsReply(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumeratePublicationsReply(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumeratePublicationsReply(const MMsgObsEnumeratePublicationsReply& theMsgR);

	// Destructor
	~MMsgObsEnumeratePublicationsReply(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumeratePublicationsReply& operator=(const MMsgObsEnumeratePublicationsReply& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumeratePublicationsReply(*this);
	}

	// Member access methods
	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long 
	GetNumberOfPublishers(void) const
	{ 
		return mPublicationList.size();
	}

	const PublicationList& 
	GetPublicationList() const
	{
		return mPublicationList;
	}

	// Get Publisher list for editing
	PublicationList& 
	GetPublicationList()
	{
		return mPublicationList;
	}

	void 
	SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

private:
	void AppendPublication(const Publication& thePublicationR);
	void ReadPublication();

private:
	ServerStatus mStatus;				// Request status
	PublicationList mPublicationList;	// List of Publications
};


}

#endif