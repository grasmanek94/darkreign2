#if !defined(MMsgRoutingGetUserList_H)
#define MMsgRoutingGetUserList_H

// MMsgRoutingGetUserList.h

#include <list>
#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// MMsgRoutingGetUserList
//
class MMsgRoutingGetUserList : public RoutingServerMessage {
public:
	enum { USERLISTTYPE_COMPLETE = 0,
		   USERLISTTYPE_BANNED   = 1,
		   USERLISTTYPE_INVITED  = 2,
		   USERLISTTYPE_MUTED    = 3,

		   USERLISTTYPE_MAX      = 3,
		   USERLISTTYPE_INVALID  = 255 };
    
	// Default ctor
    MMsgRoutingGetUserList(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetUserList(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetUserList(const MMsgRoutingGetUserList& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetUserList(void);

    // Assignment
    MMsgRoutingGetUserList& operator=(const MMsgRoutingGetUserList& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	unsigned char GetUserListType() const                        { return mUserListType; }
	void          SetUserListType(unsigned char theUserListType) { mUserListType = theUserListType; }
private:
	unsigned char mUserListType;
};


//
// MMsgRoutingGetUserListReply
//
class MMsgRoutingGetUserListReply : public MMsgRoutingStatusReply {
public:
	struct UserData {
		UserName mUserName;
	};
	typedef std::list<UserData> UserList;

    // Default ctor
    MMsgRoutingGetUserListReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetUserListReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetUserListReply(const MMsgRoutingGetUserListReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetUserListReply(void);

    // Assignment
    MMsgRoutingGetUserListReply& operator=(const MMsgRoutingGetUserListReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const UserList& GetUserList() const            { return mUserList; }
	void SetUserList(const UserList& theUserListR) { mUserList = theUserListR; }
	void AddUser(const UserData& theUserDataR)     { mUserList.push_back(theUserDataR); }
private:
	UserList mUserList;
};


// Inlines
inline TRawMsg* MMsgRoutingGetUserList::Duplicate(void) const
    { return new MMsgRoutingGetUserList(*this); }
inline TRawMsg* MMsgRoutingGetUserListReply::Duplicate(void) const
    { return new MMsgRoutingGetUserListReply(*this); }

};  // Namespace WONMsg

inline ostream& operator<<(ostream& os, const WONMsg::MMsgRoutingGetUserListReply::UserList& theUserList)
{
	WONMsg::MMsgRoutingGetUserListReply::UserList::const_iterator itr = theUserList.begin();
	for (; itr != theUserList.end(); itr++)
		os << " * " << WONCommon::WStringToString(itr->mUserName);
	return os;
}

#endif // MMsgRoutingGetUserList_H