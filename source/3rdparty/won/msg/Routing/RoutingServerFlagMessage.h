#if !defined(RoutingServerFlagMessage_H)
#define RoutingServerFlagMessage_H

// RoutingServerFlagMessage.h

#include "RoutingServerMessage.h"

namespace WONMsg {

//
// RoutingServerFlagMessage
//
class RoutingServerFlagMessage : public RoutingServerMessage {
public:
    // Default ctor
    RoutingServerFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerFlagMessage(const RoutingServerFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerFlagMessage(void);

    // Assignment
    RoutingServerFlagMessage& operator=(const RoutingServerFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool GetOnOrOffFlag() const              { return mOnOrOffFlag; }
	void SetOnOrOffFlag(bool theOnOrOffFlag) { mOnOrOffFlag = theOnOrOffFlag; }
private:
	bool     mOnOrOffFlag;
};

//
// RoutingServerClientIdFlagMessage
//
class RoutingServerClientIdFlagMessage : public RoutingServerFlagMessage {
public:
    // Default ctor
    RoutingServerClientIdFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerClientIdFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerClientIdFlagMessage(const RoutingServerClientIdFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerClientIdFlagMessage(void);

    // Assignment
    RoutingServerClientIdFlagMessage& operator=(const RoutingServerClientIdFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerClientIdFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const ClientId GetClientId() const           { return mClientId; }
	void SetClientId(const ClientId theClientId) { mClientId = theClientId; }
private:
	ClientId mClientId;
};

//
// RoutingServerClientNameFlagMessage
//
class RoutingServerClientNameFlagMessage : public RoutingServerFlagMessage {
public:
    // Default ctor
    RoutingServerClientNameFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerClientNameFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerClientNameFlagMessage(const RoutingServerClientNameFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerClientNameFlagMessage(void);

    // Assignment
    RoutingServerClientNameFlagMessage& operator=(const RoutingServerClientNameFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerClientNameFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const ClientName& GetClientName() const             { return mClientName; }
	void SetClientName(const ClientName& theClientName) { mClientName = theClientName; }
private:
	ClientName mClientName;
};

//
// RoutingServerGenericFlagMessage
//
class RoutingServerGenericFlagMessage : public RoutingServerFlagMessage {
public:
    // Default ctor
    RoutingServerGenericFlagMessage(bool flagOnOrOff =true);

    // RoutingServerMessage ctor
    explicit RoutingServerGenericFlagMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerGenericFlagMessage(const RoutingServerGenericFlagMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerGenericFlagMessage(void);

    // Assignment
    RoutingServerGenericFlagMessage& operator=(const RoutingServerGenericFlagMessage& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new RoutingServerGenericFlagMessage(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ContentType         GetContentType() const { return mContentType; }
	ClientId            GetClientId() const    { return mClientId; }
	const ClientName&   GetClientName() const  { return mClientName; }
	const unsigned long GetWONUserId() const   { return mWONUserId; }

	void SetContentType(ContentType theContentType)     { mContentType = theContentType; }
	void SetClientId(ClientId theClientId)              { mClientId = theClientId; }
	void SetClientName(const ClientName& theClientName) { mClientName = theClientName; }
	void SetWONUserId(unsigned long theWONUserId)       { mWONUserId = theWONUserId; }
private:
	ContentType   mContentType;
	ClientId      mClientId;
	ClientName    mClientName;
	unsigned long mWONUserId;
};

};  // Namespace WONMsg

#endif // RoutingServerFlagMessage_H