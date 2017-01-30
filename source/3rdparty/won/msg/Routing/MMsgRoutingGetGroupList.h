#if !defined(MMsgRoutingGetGroupList_H)
#define MMsgRoutingGetGroupList_H

// MMsgRoutingGetGroupList.h

#include <list>
#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// MMsgRoutingGetGroupList
//
class MMsgRoutingGetGroupList : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingGetGroupList(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetGroupList(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetGroupList(const MMsgRoutingGetGroupList& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetGroupList(void);

    // Assignment
    MMsgRoutingGetGroupList& operator=(const MMsgRoutingGetGroupList& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);
};

//
// MMsgRoutingGetGroupListReply
//
class MMsgRoutingGetGroupListReply : public MMsgRoutingStatusReply {
public:
	struct GroupData {
		GroupId   mGroupId;
		GroupName mGroupName;
		bool      mIsPublic;
	};
	typedef std::list<GroupData> GroupList;

    // Default ctor
    MMsgRoutingGetGroupListReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetGroupListReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetGroupListReply(const MMsgRoutingGetGroupListReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetGroupListReply(void);

    // Assignment
    MMsgRoutingGetGroupListReply& operator=(const MMsgRoutingGetGroupListReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const GroupList& GetGroupList() const             { return mGroupList; }
	void SetGroupList(const GroupList& theGroupListR) { mGroupList = theGroupListR; }
	void AddGroup(const GroupData& theGroupDataR)     { mGroupList.push_back(theGroupDataR); }
private:
	GroupList mGroupList;
};


// Inlines
inline TRawMsg* MMsgRoutingGetGroupList::Duplicate(void) const
    { return new MMsgRoutingGetGroupList(*this); }
inline TRawMsg* MMsgRoutingGetGroupListReply::Duplicate(void) const
    { return new MMsgRoutingGetGroupListReply(*this); }

};  // Namespace WONMsg

inline ostream& operator<<(ostream& os, const WONMsg::MMsgRoutingGetGroupListReply::GroupList& theGroupList)
{
	WONMsg::MMsgRoutingGetGroupListReply::GroupList::const_iterator itr = theGroupList.begin();
	for (; itr != theGroupList.end(); itr++)
	{
		os << " * " << itr->mGroupId << "," 
			        << itr->mGroupName << ","
					<< (itr->mIsPublic ? "public" : "private");
	}
	return os;
}

#endif // MMsgRoutingGetGroupList_H