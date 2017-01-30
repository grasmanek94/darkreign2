#if !defined(SMsgFactGetFileCRC_H)
#define SMsgFactGetFileCRC_H

// TMsgFactGetFileCRC.h

// Message that is used to get a process configuration from the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

	typedef std::set<unsigned char> FACT_SERV_FIELD_SET;

class SMsgFactGetFileCRC : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetFileCRC(void);

	// TMessage ctor
	explicit SMsgFactGetFileCRC(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetFileCRC(const SMsgFactGetFileCRC& theMsgR);

	// Destructor
	virtual ~SMsgFactGetFileCRC(void);

	// Assignment
	SMsgFactGetFileCRC& operator=(const SMsgFactGetFileCRC& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	const std::string& GetFilePath(void) const { return mFilePath; }
	void SetFilePath(const std::string& thePath) { mFilePath = thePath; }

protected:
	std::string         mFilePath;
};


// Inlines
inline TRawMsg* SMsgFactGetFileCRC::Duplicate(void) const
{ return new SMsgFactGetFileCRC(*this); }


};  // Namespace WONMsg

#endif