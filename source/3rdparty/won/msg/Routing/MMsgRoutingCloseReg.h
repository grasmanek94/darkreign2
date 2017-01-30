#if !defined(MMsgRoutingCloseRegistration_H)
#define MMsgRoutingCloseRegistration_H

// MMsgRoutingCloseRegistration.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingCloseRegistration : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingCloseRegistration(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingCloseRegistration(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingCloseRegistration(const MMsgRoutingCloseRegistration& theMsgR);

    // Destructor
    virtual ~MMsgRoutingCloseRegistration(void);

    // Assignment
    MMsgRoutingCloseRegistration& operator=(const MMsgRoutingCloseRegistration& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};


// Inlines
inline TRawMsg* MMsgRoutingCloseRegistration::Duplicate(void) const
    { return new MMsgRoutingCloseRegistration(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingCloseRegistration_H