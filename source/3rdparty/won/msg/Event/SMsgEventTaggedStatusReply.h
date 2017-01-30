#ifndef TMsgEventTaggedStatusReply_H
#define TMsgEventTaggedStatusReply_H

// SMsgEventTaggedStatusReply.h

#include "SMsgEventStatusReply.h"

namespace WONMsg {
class SMsgEventTaggedStatusReply : public SMsgEventStatusReply {
public:
    // Default ctor
    SMsgEventTaggedStatusReply(void);

    // SmallMessage ctor
    explicit SMsgEventTaggedStatusReply(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgEventTaggedStatusReply(const SMsgEventTaggedStatusReply& theMsgR);

    // Destructor
    virtual ~SMsgEventTaggedStatusReply(void);

    // Assignment
    SMsgEventTaggedStatusReply& operator=(const SMsgEventTaggedStatusReply& theMsgR);

    // Virtual Duplicate from SmallMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	// Dumping
	virtual void Dump(std::ostream& os) const;
    
    // Member access
	void SetTag(unsigned short theTag);
	unsigned short GetTag() const;
private:
	unsigned short mTag;
};


// Inlines
inline TRawMsg* SMsgEventTaggedStatusReply::Duplicate(void) const
    { return new SMsgEventTaggedStatusReply(*this); }

inline void SMsgEventTaggedStatusReply::SetTag(unsigned short theTag)
	{ mTag = theTag; }
inline unsigned short SMsgEventTaggedStatusReply::GetTag() const
	{ return mTag; }

};  // Namespace WONMsg

#endif // TMsgEventTaggedStatusReply_H