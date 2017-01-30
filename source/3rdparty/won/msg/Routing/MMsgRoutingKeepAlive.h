#if !defined(MMsgRoutingKeepAlive_H)
#define MMsgRoutingKeepAlive_H

// MMsgRoutingKeepAlive.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingKeepAlive : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingKeepAlive(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingKeepAlive(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingKeepAlive(const MMsgRoutingKeepAlive& theMsgR);

    // Destructor
    virtual ~MMsgRoutingKeepAlive(void);

    // Assignment
    MMsgRoutingKeepAlive& operator=(const MMsgRoutingKeepAlive& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

// Inlines
inline TRawMsg* MMsgRoutingKeepAlive::Duplicate(void) const
    { return new MMsgRoutingKeepAlive(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingKeepAlive_H