#if !defined(MMsgRoutingClientBooted_H)
#define MMsgRoutingClientBooted_H

// MMsgRoutingClientBooted.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingClientBooted : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingClientBooted(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingClientBooted(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingClientBooted(const MMsgRoutingClientBooted& theMsgR);

    // Destructor
    virtual ~MMsgRoutingClientBooted(void);

    // Assignment
    MMsgRoutingClientBooted& operator=(const MMsgRoutingClientBooted& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool GetBootOrBanFlag() const;
	void SetBootOrBanFlag(bool theBootOrBanFlag);
private:
	bool mBootOrBanFlag;
};


// Inlines
inline TRawMsg* MMsgRoutingClientBooted::Duplicate(void) const
    { return new MMsgRoutingClientBooted(*this); }

inline bool MMsgRoutingClientBooted::GetBootOrBanFlag() const
{ return mBootOrBanFlag; }
inline void MMsgRoutingClientBooted::SetBootOrBanFlag(bool theBootOrBanFlag)
{ mBootOrBanFlag = theBootOrBanFlag; }

};  // Namespace WONMsg

#endif // MMsgRoutingClientBooted_H