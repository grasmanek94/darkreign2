#if !defined(MMsgRoutingAddToGroup_H)
#define MMsgRoutingAddToGroup_H

// MMsgRoutingAddToGroup.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingAddToGroup : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingAddToGroup(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingAddToGroup(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingAddToGroup(const MMsgRoutingAddToGroup& theMsgR);

    // Destructor
    virtual ~MMsgRoutingAddToGroup(void);

    // Assignment
    MMsgRoutingAddToGroup& operator=(const MMsgRoutingAddToGroup& theMsgR);

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
inline TRawMsg* MMsgRoutingAddToGroup::Duplicate(void) const
    { return new MMsgRoutingAddToGroup(*this); }

inline GroupId MMsgRoutingAddToGroup::GetGroupId() const
{ return mGroupId; }
inline ClientId MMsgRoutingAddToGroup::GetClientId() const
{ return mClientId; }
inline void MMsgRoutingAddToGroup::SetGroupId(GroupId theGroupId)
{ mGroupId = theGroupId; }
inline void MMsgRoutingAddToGroup::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }

};  // Namespace WONMsg

#endif // MMsgRoutingAddToGroup_H