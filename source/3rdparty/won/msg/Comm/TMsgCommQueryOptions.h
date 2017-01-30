#ifndef _TMsgCommQueryOptions_H
#define _TMsgCommQueryOptions_H

// TMsgCommQueryOptions.h

// Common query option message classes.  Supports option requests and replys to
// WON servers.


#include <list>
#include <map>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommQueryOptions : public TMessage
{
public:
	// Types
	typedef std::list<std::string> OptionList;

	// Default ctor
	TMsgCommQueryOptions(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommQueryOptions(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommQueryOptions(const TMsgCommQueryOptions& theMsgR);

	// Destructor
	~TMsgCommQueryOptions(void);

	// Assignment
	TMsgCommQueryOptions& operator=(const TMsgCommQueryOptions& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	// Attribute Access
	const OptionList& GetOptionList() const;
	OptionList&       GetOptionList();

private:
	OptionList mOptionList;  // List of options to query
};


class TMsgCommQueryOptReply : public TMessage
{
public:
	// Types
	enum OptionStatus {
		OptionOk=0, OptionNotVisible=1, OptionNotSet=2
	};
	struct OptionDef { // FUCKIN' MS COMPILER: this used to be a pair, but I got some bogus compile-time errors
		OptionStatus first;
		std::wstring second;
		OptionDef() : first(OptionNotSet) {};
		OptionDef(OptionStatus aStatus, const std::wstring& aVal) : first(aStatus), second(aVal) {};
	};
	typedef std::map<std::string, OptionDef> OptionMap;

	// Default ctor
	TMsgCommQueryOptReply(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgCommQueryOptReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgCommQueryOptReply(const TMsgCommQueryOptReply& theMsgR);

	// Destructor
	~TMsgCommQueryOptReply(void);

	// Assignment
	TMsgCommQueryOptReply& operator=(const TMsgCommQueryOptReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	// Status access
	ServerStatus GetStatus(void) const;
	void         SetStatus(ServerStatus theStatus);
	const OptionMap& GetOptionMap() const;
	OptionMap&       GetOptionMap();

private:
	ServerStatus mStatus;  // Request status
	OptionMap mOptionMap;  // Map of returned options
};


// Inlines
inline TRawMsg*
TMsgCommQueryOptions::Duplicate(void) const
{ return new TMsgCommQueryOptions(*this); }

inline TRawMsg*
TMsgCommQueryOptReply::Duplicate(void) const
{ return new TMsgCommQueryOptReply(*this); }

inline ServerStatus
TMsgCommQueryOptReply::GetStatus(void) const
{ return mStatus; }

inline void
TMsgCommQueryOptReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const TMsgCommQueryOptions::OptionList&
TMsgCommQueryOptions::GetOptionList() const
{ return mOptionList; }

inline TMsgCommQueryOptions::OptionList&
TMsgCommQueryOptions::GetOptionList()
{ return mOptionList; }

inline const TMsgCommQueryOptReply::OptionMap&
TMsgCommQueryOptReply::GetOptionMap() const
{ return mOptionMap; }

inline TMsgCommQueryOptReply::OptionMap&
TMsgCommQueryOptReply::GetOptionMap()
{ return mOptionMap; }

};  // Namespace WONMsg

#endif