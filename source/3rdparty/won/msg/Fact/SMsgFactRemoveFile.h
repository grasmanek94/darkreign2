#if !defined(SMsgFactRemoveFile_H)
#define SMsgFactRemoveFile_H

// TMsgFactRemoveFile.h

// Message that is used to get a process configuration from the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

class SMsgFactRemoveFile : public SmallMessage {

public:
	// Default ctor
	SMsgFactRemoveFile(void);

	// TMessage ctor
	explicit SMsgFactRemoveFile(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactRemoveFile(const SMsgFactRemoveFile& theMsgR);

	// Destructor
	virtual ~SMsgFactRemoveFile(void);

	// Assignment
	SMsgFactRemoveFile& operator=(const SMsgFactRemoveFile& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactRemoveFile(*this); }

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	void SetFilePath(const std::string& theFilePath) { mFilePath = theFilePath; }
	const std::string& GetFilePath() const { return mFilePath; }

protected:
	std::string         mFilePath;

};


};  // Namespace WONMsg

#endif