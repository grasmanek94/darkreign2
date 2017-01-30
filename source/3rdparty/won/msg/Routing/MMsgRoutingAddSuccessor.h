#if !defined(MMsgRoutingAddSuccessor_H)
#define MMsgRoutingAddSuccessor_H

// MMsgRoutingAddSuccessor.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingAddSuccessor : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingAddSuccessor(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingAddSuccessor(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingAddSuccessor(const MMsgRoutingAddSuccessor& theMsgR);

    // Destructor
    virtual ~MMsgRoutingAddSuccessor(void);

    // Assignment
    MMsgRoutingAddSuccessor& operator=(const MMsgRoutingAddSuccessor& theMsgR);

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
	
	unsigned char GetRanking() const;
	void          SetRanking(unsigned char theRanking);
private:
	ClientId      mClientId;
	unsigned char mRanking;
};


// Inlines
inline TRawMsg* MMsgRoutingAddSuccessor::Duplicate(void) const
    { return new MMsgRoutingAddSuccessor(*this); }

inline ClientId MMsgRoutingAddSuccessor::GetClientId() const
{ return mClientId; }
inline void MMsgRoutingAddSuccessor::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }
inline unsigned char MMsgRoutingAddSuccessor::GetRanking() const
{ return mRanking; }
inline void MMsgRoutingAddSuccessor::SetRanking(unsigned char theRanking)
{ mRanking = theRanking; }

};  // Namespace WONMsg

#endif // MMsgRoutingAddSuccessor_H