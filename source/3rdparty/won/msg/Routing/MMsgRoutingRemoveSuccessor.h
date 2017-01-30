#if !defined(MMsgRoutingRemoveSuccessor_H)
#define MMsgRoutingRemoveSuccessor_H

// MMsgRoutingRemoveSuccessor.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingRemoveSuccessor : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingRemoveSuccessor(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingRemoveSuccessor(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingRemoveSuccessor(const MMsgRoutingRemoveSuccessor& theMsgR);

    // Destructor
    virtual ~MMsgRoutingRemoveSuccessor(void);

    // Assignment
    MMsgRoutingRemoveSuccessor& operator=(const MMsgRoutingRemoveSuccessor& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientId GetClientId() const;
	void     SetClientId(ClientId theClientId);
private:
	ClientId      mClientId;
};


// Inlines
inline TRawMsg* MMsgRoutingRemoveSuccessor::Duplicate(void) const
    { return new MMsgRoutingRemoveSuccessor(*this); }

inline ClientId MMsgRoutingRemoveSuccessor::GetClientId() const
{ return mClientId; }
inline void MMsgRoutingRemoveSuccessor::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }

};  // Namespace WONMsg

#endif // MMsgRoutingRemoveSuccessor_H