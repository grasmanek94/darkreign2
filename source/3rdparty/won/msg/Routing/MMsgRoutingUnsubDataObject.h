#if !defined(MMsgRoutingUnsubscribeDataObject_H)
#define MMsgRoutingUnsubscribeDataObject_H

// MMsgRoutingUnsubscribeDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingUnsubscribeDataObject
//
class MMsgRoutingUnsubscribeDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingUnsubscribeDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingUnsubscribeDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingUnsubscribeDataObject(const MMsgRoutingUnsubscribeDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingUnsubscribeDataObject(void);

    // Assignment
    MMsgRoutingUnsubscribeDataObject& operator=(const MMsgRoutingUnsubscribeDataObject& theMsgR);

    // Virtual Duplicate from MMsgRoutingBaseDataObject
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool GetExactOrRecursiveFlag() const                       { return mExactOrRecursiveFlag; }
	bool GetGroupOrMembersFlag() const                         { return mGroupOrMembersFlag; }

	void SetExactOrRecursiveFlag(bool theExactOrRecursiveFlag) { mExactOrRecursiveFlag = theExactOrRecursiveFlag; }
	void SetGroupOrMembersFlag(bool theGroupOrMembersFlag)     { mGroupOrMembersFlag = theGroupOrMembersFlag; }
private:
	bool mExactOrRecursiveFlag;
	bool mGroupOrMembersFlag;
};

// Inlines
inline TRawMsg* MMsgRoutingUnsubscribeDataObject::Duplicate(void) const
    { return new MMsgRoutingUnsubscribeDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingUnsubscribeDataObject_H