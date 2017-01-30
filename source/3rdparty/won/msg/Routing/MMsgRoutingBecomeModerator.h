#if !defined(MMsgRoutingBecomeModerator_H)
#define MMsgRoutingBecomeModerator_H

// MMsgRoutingBecomeModerator.h

#include "RoutingServerFlagMessage.h"

namespace WONMsg {

class MMsgRoutingBecomeModerator : public RoutingServerClientIdFlagMessage {
public:
    // Default ctor
    MMsgRoutingBecomeModerator(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit MMsgRoutingBecomeModerator(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingBecomeModerator(const MMsgRoutingBecomeModerator& theMsgR);

    // Destructor
    virtual ~MMsgRoutingBecomeModerator(void);

    // Assignment
    MMsgRoutingBecomeModerator& operator=(const MMsgRoutingBecomeModerator& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingBecomeModerator(*this); }

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

};  // Namespace WONMsg

#endif // MMsgRoutingBecomeModerator_H