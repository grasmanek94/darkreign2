#if !defined(MMsgRoutingSpectatorCount_H)
#define MMsgRoutingSpectatorCount_H

// MMsgRoutingSpectatorCount.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingSpectatorCount : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingSpectatorCount(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSpectatorCount(const RoutingServerMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    MMsgRoutingSpectatorCount(const MMsgRoutingSpectatorCount& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSpectatorCount(void);

    // Assignment
    MMsgRoutingSpectatorCount& operator=(const MMsgRoutingSpectatorCount& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	unsigned short GetSpectatorCount() const                  { return mSpectatorCount; }
	void  SetSpectatorCount(unsigned short theSpectatorCount) { mSpectatorCount = theSpectatorCount; }
protected:
	unsigned short mSpectatorCount;
};


// Inlines
inline TRawMsg* MMsgRoutingSpectatorCount::Duplicate(void) const
    { return new MMsgRoutingSpectatorCount(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingSpectatorCount_H