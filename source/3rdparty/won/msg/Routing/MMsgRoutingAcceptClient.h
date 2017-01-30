#if !defined(MMsgRoutingAcceptClient_H)
#define MMsgRoutingAcceptClient_H

// MMsgRoutingAcceptClient.h

#include "RoutingServerMessage.h"

namespace WONMsg {

//
// MMsgRoutingAcceptClient
//
class MMsgRoutingAcceptClient : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingAcceptClient(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingAcceptClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingAcceptClient(const MMsgRoutingAcceptClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingAcceptClient(void);

    // Assignment
    MMsgRoutingAcceptClient& operator=(const MMsgRoutingAcceptClient& theMsgR);

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
    const ClientName& GetClientName() const              { return mClientName; }
    void SetClientName(const ClientName& theClientNameR) { mClientName = theClientNameR; }
private:
	ClientName mClientName;
};

//
// MMsgRoutingAcceptClientReply
//
class MMsgRoutingAcceptClientReply : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingAcceptClientReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingAcceptClientReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingAcceptClientReply(const MMsgRoutingAcceptClientReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingAcceptClientReply(void);

    // Assignment
    MMsgRoutingAcceptClientReply& operator=(const MMsgRoutingAcceptClientReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
    const ClientName& GetClientName() const                { return mClientName; }
    bool GetAcceptOrRejectFlag() const                     { return mAcceptOrRejectFlag; }
    const std::wstring& GetComment() const                 { return mComment; }

    void SetClientName(const ClientName& theClientNameR)         { mClientName = theClientNameR; }
    void SetAcceptOrRejectFlag(bool theAcceptOrRejectFlag) { mAcceptOrRejectFlag = theAcceptOrRejectFlag; }
    void SetComment(const std::wstring& theCommentR)       { mComment = theCommentR; }
private:
	ClientName     mClientName;
	bool         mAcceptOrRejectFlag;
	std::wstring mComment;
};


// Inlines
inline TRawMsg* MMsgRoutingAcceptClient::Duplicate(void) const
    { return new MMsgRoutingAcceptClient(*this); }
inline TRawMsg* MMsgRoutingAcceptClientReply::Duplicate(void) const
    { return new MMsgRoutingAcceptClientReply(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingAcceptClient_H