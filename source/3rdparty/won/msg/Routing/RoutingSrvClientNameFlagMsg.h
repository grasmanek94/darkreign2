#if !defined(ClientNameFlagMessage_H)
#define ClientNameFlagMessage_H

// RoutingServerClientNameFlagMessage.h

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class RoutingServerClientNameFlagMessage : public RoutingServerFlagMessage {
public:
    // Default ctor
    RoutingServerClientNameFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerClientNameFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerClientNameFlagMessage(const RoutingServerClientNameFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerClientNameFlagMessage(void);

    // Assignment
    RoutingServerClientNameFlagMessage& operator=(const RoutingServerClientNameFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerClientNameFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const ClientName& GetClientName() const             { return mClientName; }
	void SetClientName(const ClientName& theClientName) { mClientName = theClientName; }
private:
	ClientName mClientName;
};

};  // Namespace WONMsg

#endif // ClientName6FlagMessage_H