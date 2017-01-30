#if !defined(MMsgRoutingDeleteGroup_H)
#define MMsgRoutingDeleteGroup_H

// MMsgRoutingDeleteGroup.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingDeleteGroup : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingDeleteGroup(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingDeleteGroup(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingDeleteGroup(const MMsgRoutingDeleteGroup& theMsgR);

    // Destructor
    virtual ~MMsgRoutingDeleteGroup(void);

    // Assignment
    MMsgRoutingDeleteGroup& operator=(const MMsgRoutingDeleteGroup& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	GroupId GetGroupId() const;

	void SetGroupId(GroupId theGroupId);
private:
	GroupId mGroupId;
};


// Inlines
inline TRawMsg* MMsgRoutingDeleteGroup::Duplicate(void) const
    { return new MMsgRoutingDeleteGroup(*this); }

inline GroupId MMsgRoutingDeleteGroup::GetGroupId() const
{ return mGroupId; }
inline void MMsgRoutingDeleteGroup::SetGroupId(GroupId theGroupId)
{ mGroupId = theGroupId; }

};  // Namespace WONMsg

#endif // MMsgRoutingDeleteGroup_H