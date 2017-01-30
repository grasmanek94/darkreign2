#if !defined(MMsgRoutingInviteClient_H)
#define MMsgRoutingInviteClient_H

// MMsgRoutingInviteClient.h

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class MMsgRoutingInviteClient : public RoutingServerClientNameFlagMessage {
public:
    // Default ctor
    MMsgRoutingInviteClient(bool flagInviteOrUninvite =true);

    // RoutingServerMessage ctor
    explicit MMsgRoutingInviteClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingInviteClient(const MMsgRoutingInviteClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingInviteClient(void);

    // Assignment
    MMsgRoutingInviteClient& operator=(const MMsgRoutingInviteClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingInviteClient(*this); }

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

};  // Namespace WONMsg

#endif // MMsgRoutingInviteClient_H