#if !defined(MMsgRoutingBanClient_H)
#define MMsgRoutingBanClient_H

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class MMsgRoutingBanClient : public RoutingServerGenericFlagMessage {
public:
    // Default ctor
    MMsgRoutingBanClient(bool flagBanOrUnban =true);

    // RoutingServerMessage ctor
    explicit MMsgRoutingBanClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingBanClient(const MMsgRoutingBanClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingBanClient(void);

    // Assignment
    MMsgRoutingBanClient& operator=(const MMsgRoutingBanClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingBanClient(*this); }

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

};  // Namespace WONMsg

#endif // MMsgRoutingBanClient_H