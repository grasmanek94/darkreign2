#if !defined(SMsgCommDumpStatistics_H)
#define SMsgCommDumpStatistics_H

#include "msg/TMessage.h"

namespace WONMsg {

class SMsgCommDumpStatistics : public SmallMessage {
public:
	// Default ctor
	SMsgCommDumpStatistics();

	// TMessage ctor
	explicit SMsgCommDumpStatistics(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgCommDumpStatistics(const SMsgCommDumpStatistics& theMsgR);

	// Destructor
	virtual ~SMsgCommDumpStatistics(void);

	// Assignment
	SMsgCommDumpStatistics& operator=(const SMsgCommDumpStatistics& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const { return new SMsgCommDumpStatistics(*this); }

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);
};

};  // Namespace WONMsg

#endif // SMsgCommDumpStatistics_H