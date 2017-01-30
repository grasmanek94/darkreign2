#if !defined(SMsgFactProcessListReply_H)
#define SMsgFactProcessListReply_H

// TMsgFactProcessListReply.h

// Message that is used to return a list of process configuration names from the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

	typedef std::set<std::string> FACT_SERV_PROCESS_SET;


class SMsgFactProcessListReply : public SmallMessage {

public:
	// Default ctor
	SMsgFactProcessListReply(void);

	// TMessage ctor
	explicit SMsgFactProcessListReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactProcessListReply(const SMsgFactProcessListReply& theMsgR);

	// Destructor
	virtual ~SMsgFactProcessListReply(void);

	// Assignment
	SMsgFactProcessListReply& operator=(const SMsgFactProcessListReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	short GetStatus(void) const;
	const FACT_SERV_PROCESS_SET& GetProcessSet(void) const;

	virtual void SetStatus(short theStatus);
	virtual void SetProcessSet(const FACT_SERV_PROCESS_SET& theProcessSet);

protected:
	short                 mStatus;
	FACT_SERV_PROCESS_SET mProcessSet;

};


// Inlines
inline TRawMsg* SMsgFactProcessListReply::Duplicate(void) const
{ return new SMsgFactProcessListReply(*this); }

inline short SMsgFactProcessListReply::GetStatus(void) const
{ return mStatus; }

inline const FACT_SERV_PROCESS_SET& SMsgFactProcessListReply::GetProcessSet(void) const
{ return mProcessSet; }

inline void SMsgFactProcessListReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

inline void SMsgFactProcessListReply::SetProcessSet(const FACT_SERV_PROCESS_SET& theProcessSet)
{ mProcessSet = theProcessSet; }

};  // Namespace WONMsg

#endif