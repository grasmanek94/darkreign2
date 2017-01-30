#if !defined(MMsgRoutingGetSimpleClientList_H)
#define MMsgRoutingGetSimpleClientList_H

// MMsgRoutingGetSimpleClientList.h

#include <list>
#include <climits>
#include <common/OutputOperators.h>
#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

enum ClientListType { CLIENTLISTTYPE_COMPLETE = 0,
	                  CLIENTLISTTYPE_BANNED   = 1,
	                  CLIENTLISTTYPE_INVITED  = 2,
	                  CLIENTLISTTYPE_MUTED    = 3,
                      //
	                  CLIENTLISTTYPE_MAX      = 3,
	                  CLIENTLISTTYPE_INVALID  = 255 };

//
// MMsgRoutingGetSimpleClientList
//
class MMsgRoutingGetSimpleClientList : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingGetSimpleClientList(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetSimpleClientList(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetSimpleClientList(const MMsgRoutingGetSimpleClientList& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetSimpleClientList(void);

    // Assignment
    MMsgRoutingGetSimpleClientList& operator=(const MMsgRoutingGetSimpleClientList& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetSimpleClientList(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

	unsigned char GetClientListType() const                          { return mClientListType; }
	void          SetClientListType(unsigned char theClientListType) { mClientListType = theClientListType; }
private:
	unsigned char mClientListType;
};

//
// MMsgRoutingGetSimpleClientListReply
//
class MMsgRoutingGetSimpleClientListReply : public MMsgRoutingStatusReply {
public:
	typedef std::list<unsigned long> WONUserIdList;
	typedef std::list<ClientId>      ClientIdList;
	enum { MAX_CLIENT_LIST_SIZE = USHRT_MAX };

    // Default ctor
    MMsgRoutingGetSimpleClientListReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetSimpleClientListReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetSimpleClientListReply(const MMsgRoutingGetSimpleClientListReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetSimpleClientListReply(void);

    // Assignment
    MMsgRoutingGetSimpleClientListReply& operator=(const MMsgRoutingGetSimpleClientListReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetSimpleClientListReply(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	unsigned char GetClientListType() const                          { return mClientListType; }
	void          SetClientListType(unsigned char theClientListType) { mClientListType = theClientListType; }

	const ClientIdList&   GetClientIdList() const   { return mClientIdList; }
	const ClientNameList& GetClientNameList() const { return mClientNameList; }
	const WONUserIdList&  GetWONUserIdList() const  { return mWONUserIdList; }

	void SetClientIdList(const ClientIdList& theClientIdListR)       { mClientIdList = theClientIdListR; }
	void SetClientNameList(const ClientNameList& theClientNameListR) { mClientNameList = theClientNameListR; }
	void SetWONUserIdList(const WONUserIdList& theWONUserIdListR)    { mWONUserIdList = theWONUserIdListR; }
	
	void AddClientId(ClientId theClientId)               { mClientIdList.push_back(theClientId); }
	void AddClientName(const ClientName& theClientNameR) { mClientNameList.push_back(theClientNameR); }
	void AddWONUserId(unsigned long theWONUserId)        { mWONUserIdList.push_back(theWONUserId); }
private:
	unsigned char  mClientListType;
	ClientIdList   mClientIdList;
	ClientNameList mClientNameList;
	WONUserIdList  mWONUserIdList;
};

};  // Namespace WONMsg

#endif // MMsgRoutingGetSimpleClientList_H