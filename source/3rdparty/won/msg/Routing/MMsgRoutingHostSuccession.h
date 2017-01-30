#if !defined(MMsgRoutingHostSuccessionInProgress_H)
#define MMsgRoutingHostSuccessionInProgress_H

// MMsgRoutingHostSuccessionInProgress.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingHostSuccessionInProgress : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingHostSuccessionInProgress(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingHostSuccessionInProgress(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingHostSuccessionInProgress(const MMsgRoutingHostSuccessionInProgress& theMsgR);

    // Destructor
    virtual ~MMsgRoutingHostSuccessionInProgress(void);

    // Assignment
    MMsgRoutingHostSuccessionInProgress& operator=(const MMsgRoutingHostSuccessionInProgress& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};


// Inlines
inline TRawMsg* MMsgRoutingHostSuccessionInProgress::Duplicate(void) const
    { return new MMsgRoutingHostSuccessionInProgress(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingHostSuccessionInProgress_H