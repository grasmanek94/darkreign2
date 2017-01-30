#if !defined(MMsgRoutingBootClient_H)
#define MMsgRoutingBootClient_H

// MMsgRoutingBootClient.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingBootClient : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingBootClient(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingBootClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingBootClient(const MMsgRoutingBootClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingBootClient(void);

    // Assignment
    MMsgRoutingBootClient& operator=(const MMsgRoutingBootClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingBootClient(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientId GetClientId() const           { return mClientId; }
	void SetClientId(ClientId theClientId) { mClientId = theClientId; }
private:
	ClientId mClientId;
};

};  // Namespace WONMsg

#endif // MMsgRoutingBootClient_H