#if !defined(MMsgRoutingMuteClient_H)
#define MMsgRoutingMuteClient_H

// MMsgRoutingMuteClient.h

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class MMsgRoutingMuteClient : public RoutingServerClientIdFlagMessage {
public:
    // Default ctor
    MMsgRoutingMuteClient(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit MMsgRoutingMuteClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingMuteClient(const MMsgRoutingMuteClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingMuteClient(void);

    // Assignment
    MMsgRoutingMuteClient& operator=(const MMsgRoutingMuteClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingMuteClient(*this); }

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

};  // Namespace WONMsg

#endif // MMsgRoutingMuteClient_H