#if !defined(MMsgRoutingGroupSpectatorCount_H)
#define MMsgRoutingGroupSpectatorCount_H

// MMsgRoutingGroupSpectatorCount.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingGroupSpectatorCount : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingGroupSpectatorCount(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGroupSpectatorCount(const RoutingServerMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    MMsgRoutingGroupSpectatorCount(const MMsgRoutingGroupSpectatorCount& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGroupSpectatorCount(void);

    // Assignment
    MMsgRoutingGroupSpectatorCount& operator=(const MMsgRoutingGroupSpectatorCount& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	GroupId GetGroupId() const                      { return mGroupId; }
	short   GetSpectatorCount() const               { return mSpectatorCount; }

	void SetGroupId(GroupId theGroupId)             { mGroupId = theGroupId; }
	void SetSpectatorCount(short theSpectatorCount) { mSpectatorCount = theSpectatorCount; }
protected:
	GroupId        mGroupId;
	unsigned short mSpectatorCount;
};


// Inlines
inline TRawMsg* MMsgRoutingGroupSpectatorCount::Duplicate(void) const
    { return new MMsgRoutingGroupSpectatorCount(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingGroupSpectatorCount_H