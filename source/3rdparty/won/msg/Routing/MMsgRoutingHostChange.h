#if !defined(MMsgRoutingHostChange_H)
#define MMsgRoutingHostChange_H

// MMsgRoutingHostChange.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingHostChange : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingHostChange(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingHostChange(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingHostChange(const MMsgRoutingHostChange& theMsgR);

    // Destructor
    virtual ~MMsgRoutingHostChange(void);

    // Assignment
    MMsgRoutingHostChange& operator=(const MMsgRoutingHostChange& theMsgR);

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
	ClientId mClientId;
};


// Inlines
inline TRawMsg* MMsgRoutingHostChange::Duplicate(void) const
    { return new MMsgRoutingHostChange(*this); }

inline ClientId MMsgRoutingHostChange::GetClientId() const
{ return mClientId; }
inline void MMsgRoutingHostChange::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }

};  // Namespace WONMsg

#endif // MMsgRoutingHostChange_H