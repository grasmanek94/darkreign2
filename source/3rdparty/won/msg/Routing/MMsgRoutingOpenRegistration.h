#if !defined(MMsgRoutingOpenRegistration_H)
#define MMsgRoutingOpenRegistration_H

// MMsgRoutingOpenRegistration.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingOpenRegistration : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingOpenRegistration(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingOpenRegistration(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingOpenRegistration(const MMsgRoutingOpenRegistration& theMsgR);

    // Destructor
    virtual ~MMsgRoutingOpenRegistration(void);

    // Assignment
    MMsgRoutingOpenRegistration& operator=(const MMsgRoutingOpenRegistration& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};


// Inlines
inline TRawMsg* MMsgRoutingOpenRegistration::Duplicate(void) const
    { return new MMsgRoutingOpenRegistration(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingOpenRegistration_H