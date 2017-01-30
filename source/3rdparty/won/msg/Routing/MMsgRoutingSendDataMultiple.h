#if !defined(MMsgRoutingSendDataMultiple_H)
#define MMsgRoutingSendDataMultiple_H

// MMsgRoutingSendDataMultiple.h

#include "common/won.h"
#include "AddresseeList.h"

namespace WONMsg {

class MMsgRoutingSendDataMultiple : public RoutingServerMessage, public AddresseeList {
public:
    // Default ctor
    MMsgRoutingSendDataMultiple(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSendDataMultiple(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSendDataMultiple(const MMsgRoutingSendDataMultiple& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSendDataMultiple(void);

    // Assignment
    MMsgRoutingSendDataMultiple& operator=(const MMsgRoutingSendDataMultiple& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingSendDataMultiple(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	typedef std::list<WONCommon::RawBuffer> MessageList;

    // Member access
	bool ShouldSendReply() const                             { return mShouldSendReply; }
	const MessageList& GetMessageList() const                { return mMessageList; }

	void SetShouldSendReply(bool shouldSendReply)            { mShouldSendReply = shouldSendReply; }
	void SetMessageList(const MessageList& theMessageListR)  { mMessageList = theMessageListR; }
	void AddMessage(const WONCommon::RawBuffer& theMessageR) { mMessageList.push_back(theMessageR); }
private:
	bool        mShouldSendReply;
	MessageList mMessageList;
};

}; // Namespace WONMsg

#endif // MMsgRoutingSendDataMultiple_H