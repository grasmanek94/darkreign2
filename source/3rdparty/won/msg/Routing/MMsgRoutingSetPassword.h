#if !defined(MMsgRoutingSetPassword_H)
#define MMsgRoutingSetPassword_H

// MMsgRoutingSetPassword.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingSetPassword : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingSetPassword(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSetPassword(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSetPassword(const MMsgRoutingSetPassword& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSetPassword(void);

    // Assignment
    MMsgRoutingSetPassword& operator=(const MMsgRoutingSetPassword& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const Password& GetNewPassword() const;

	void SetNewPassword(const Password& theNewPassword);
private:
	Password mNewPassword;
};


// Inlines
inline TRawMsg* MMsgRoutingSetPassword::Duplicate(void) const
    { return new MMsgRoutingSetPassword(*this); }

inline const Password& MMsgRoutingSetPassword::GetNewPassword() const
{ return mNewPassword; }
inline void MMsgRoutingSetPassword::SetNewPassword(const Password& theNewPassword)
{ mNewPassword = theNewPassword; }

};  // Namespace WONMsg

#endif // MMsgRoutingSetPassword_H