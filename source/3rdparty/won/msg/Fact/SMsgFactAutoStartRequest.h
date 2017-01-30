#ifndef SMSGFACTAUTOSTARTREQUEST_H
#define SMSGFACTAUTOSTARTREQUEST_H

#include "msg/TMessage.h"

namespace WONMsg {

//
// SMsgFactAutoStartRequest
//
class SMsgFactAutoStartRequest : public SmallMessage {
public:
	// Default ctor
	SMsgFactAutoStartRequest(void);

	// Unpacking ctor
	SMsgFactAutoStartRequest(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactAutoStartRequest(const SMsgFactAutoStartRequest& theMsgR);

	// Destructor
	virtual ~SMsgFactAutoStartRequest(void);

	// Assignment
	SMsgFactAutoStartRequest& operator=(const SMsgFactAutoStartRequest& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactAutoStartRequest(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
protected:
};

}; // namespace WONMsg

#endif // SMSGFACTAUTOSTARTREQUEST_H
