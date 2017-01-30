#if !defined(MMsgRoutingRemoveFromGroup_H)
#define MMsgRoutingRemoveFromGroup_H

// MMsgRoutingRemoveFromGroup.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingRemoveFromGroup : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingRemoveFromGroup(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingRemoveFromGroup(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingRemoveFromGroup(const MMsgRoutingRemoveFromGroup& theMsgR);

    // Destructor
    virtual ~MMsgRoutingRemoveFromGroup(void);

    // Assignment
    MMsgRoutingRemoveFromGroup& operator=(const MMsgRoutingRemoveFromGroup& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	GroupId  GetGroupId() const;
	ClientId GetClientId() const;

	void SetGroupId(GroupId theGroupId);
	void SetClientId(ClientId theClientId);
private:
	GroupId  mGroupId;
	ClientId mClientId;
};


// Inlines
inline TRawMsg* MMsgRoutingRemoveFromGroup::Duplicate(void) const
    { return new MMsgRoutingRemoveFromGroup(*this); }

inline GroupId MMsgRoutingRemoveFromGroup::GetGroupId() const
{ return mGroupId; }
inline ClientId MMsgRoutingRemoveFromGroup::GetClientId() const
{ return mClientId; }
inline void MMsgRoutingRemoveFromGroup::SetGroupId(GroupId theGroupId)
{ mGroupId = theGroupId; }
inline void MMsgRoutingRemoveFromGroup::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }

};  // Namespace WONMsg

#endif // MMsgRoutingRemoveFromGroup_H