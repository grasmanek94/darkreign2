#if !defined(MMsgRoutingCreateGroup_H)
#define MMsgRoutingCreateGroup_H

// MMsgRoutingCreateGroup.h

#include <list>
#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// MMsgRoutingCreateGroup
//
class MMsgRoutingCreateGroup : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingCreateGroup(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingCreateGroup(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingCreateGroup(const MMsgRoutingCreateGroup& theMsgR);

    // Destructor
    virtual ~MMsgRoutingCreateGroup(void);

    // Assignment
    MMsgRoutingCreateGroup& operator=(const MMsgRoutingCreateGroup& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const GroupName& GetGroupName() const                         { return mGroupName; }
	bool IsPublic() const                                         { return mIsPublic; }
	bool AnnounceGroupChanges() const                             { return mAnnounceGroupChanges; }
	const std::list<ClientId>& GetClientList() const              { return mClientList; }

	void SetGroupName(const GroupName& theGroupName)              { mGroupName = theGroupName; }
	void SetIsPublic(bool isPublic)                               { mIsPublic = isPublic; }
	void SetAnnounceGroupChanges(bool announceGroupChanges)       { mAnnounceGroupChanges = announceGroupChanges; }
	void SetClientList(const std::list<ClientId>& theClientListR) { mClientList = theClientListR; }
	void AddClient(ClientId theClientId)                          { mClientList.push_back(theClientId); }
private:
	GroupName           mGroupName;
	bool                mIsPublic;
	bool                mAnnounceGroupChanges;
	std::list<ClientId> mClientList;
};


//
// MMsgRoutingCreateGroupReply
//
class MMsgRoutingCreateGroupReply : public MMsgRoutingStatusReply {
public:
    // Default ctor
    MMsgRoutingCreateGroupReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingCreateGroupReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingCreateGroupReply(const MMsgRoutingCreateGroupReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingCreateGroupReply(void);

    // Assignment
    MMsgRoutingCreateGroupReply& operator=(const MMsgRoutingCreateGroupReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	GroupId GetGroupId() const             { return mGroupId; }
	void    SetGroupId(GroupId theGroupId) { mGroupId = theGroupId; }
private:
	GroupId mGroupId;
};

// Inlines
inline TRawMsg* MMsgRoutingCreateGroup::Duplicate(void) const
    { return new MMsgRoutingCreateGroup(*this); }
inline TRawMsg* MMsgRoutingCreateGroupReply::Duplicate(void) const
    { return new MMsgRoutingCreateGroupReply(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingCreateGroup_H