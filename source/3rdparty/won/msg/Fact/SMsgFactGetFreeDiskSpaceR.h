#if !defined(SMsgFactGetFreeDiskSpaceReply_H)
#define SMsgFactGetFreeDiskSpaceReply_H

// TMsgFactGetFreeDiskSpaceReply.h

// Message that is used to get a list of process configurations from the Factory Server


#pragma warning(disable:4786)
#include <string>
#include <list>
#include <vector>
#include "msg/TMessage.h"

namespace WONMsg {

class SMsgFactGetFreeDiskSpaceReply : public SmallMessage {
public:
	struct FreeDiskSpace
	{
		__int64 mTotal;
		__int64 mFree;
	};

	typedef std::pair<std::string, FreeDiskSpace> FreeDiskSpacePair;
	typedef std::list<FreeDiskSpacePair> FreeDiskSpaceList;

public:
	// Default ctor
	SMsgFactGetFreeDiskSpaceReply(void);

	// TMessage ctor
	explicit SMsgFactGetFreeDiskSpaceReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetFreeDiskSpaceReply(const SMsgFactGetFreeDiskSpaceReply& theMsgR);

	// Destructor
	virtual ~SMsgFactGetFreeDiskSpaceReply(void);

	// Assignment
	SMsgFactGetFreeDiskSpaceReply& operator=(const SMsgFactGetFreeDiskSpaceReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access

	FreeDiskSpaceList& GetFreeDiskSpaceList() { return mFreeDiskSpaceList; }


protected:
	FreeDiskSpaceList mFreeDiskSpaceList;
};


// Inlines
inline TRawMsg* SMsgFactGetFreeDiskSpaceReply::Duplicate(void) const
{ return new SMsgFactGetFreeDiskSpaceReply(*this); }


};  // Namespace WONMsg


#endif