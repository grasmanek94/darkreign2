#if !defined(ClientIdFlagMessage_H)
#define ClientIdFlagMessage_H

// RoutingServerClientIdFlagMessage.h

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class RoutingServerClientIdFlagMessage : public RoutingServerFlagMessage {
public:
    // Default ctor
    RoutingServerClientIdFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerClientIdFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerClientIdFlagMessage(const RoutingServerClientIdFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerClientIdFlagMessage(void);

    // Assignment
    RoutingServerClientIdFlagMessage& operator=(const RoutingServerClientIdFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerClientIdFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const ClientId GetClientId() const           { return mClientId; }
	void SetClientId(const ClientId theClientId) { mClientId = theClientId; }
private:
	ClientId mClientId;
};

};  // Namespace WONMsg

#endif // ClientIdFlagMessage_H