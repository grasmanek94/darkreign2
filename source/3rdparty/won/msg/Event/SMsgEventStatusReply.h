#ifndef TMsgEventStatusReply_H
#define TMsgEventStatusReply_H

// SMsgEventStatusReply.h

#include "msg/TMessage.h"

namespace WONMsg {
class SMsgEventStatusReply : public SmallMessage {
public:
    // Default ctor
    SMsgEventStatusReply(void);

    // SmallMessage ctor
    explicit SMsgEventStatusReply(const SmallMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    SMsgEventStatusReply(const SMsgEventStatusReply& theMsgR);

    // Destructor
    virtual ~SMsgEventStatusReply(void);

    // Assignment
    SMsgEventStatusReply& operator=(const SMsgEventStatusReply& theMsgR);

    // Virtual Duplicate from SmallMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	// Dumping
	virtual void Dump(std::ostream& os) const;
    
    // Member access
	void SetStatus(short theStatus);
	short GetStatus() const;
protected:
	short mStatus;
};


// Inlines
inline TRawMsg* SMsgEventStatusReply::Duplicate(void) const
    { return new SMsgEventStatusReply(*this); }
inline void SMsgEventStatusReply::SetStatus(short theStatus)
	{ mStatus = theStatus; }
inline short SMsgEventStatusReply::GetStatus() const
	{ return mStatus; }

};  // Namespace WONMsg

#endif // TMsgEventStatusReply_H