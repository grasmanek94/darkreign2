#if !defined(SMsgFactProcessConfigurationReply_H)
#define SMsgFactProcessConfigurationReply_H

// TMsgFactProcessConfigurationReply.h

// Message that is used to return a process configuration from the Factory Server


#include "msg/TMessage.h"
#include <map>


namespace WONMsg {

	enum FactConfigField
	{
		FactConfigDescription = 0,
		FactConfigExecutable = 1,
		FactConfigWorkingDirectory = 2,
		FactConfigCommandLine = 3,
		FactConfigStartTime = 4,
		FactConfigFirstPort = 5,
		FactConfigLastPort = 6,
		FactConfigTerminateMessage = 7,
		FactConfigStopTime = 8,
		FactConfigLengthFieldSize = 9,
		FactConfigStartAccess = 10,
		FactConfigStopAccess = 11,
		FactConfigAuthenticated = 12,
		FactConfigPingable = 13,
		FactConfigInitEvent = 14,
		FactConfigCreateConsole = 15
	};

	typedef std::map<int, string> FACT_SERV_FIELD_REPLY_MAP;


class SMsgFactProcessConfigurationReply : public SmallMessage {

public:
	// Default ctor
	SMsgFactProcessConfigurationReply(void);

	// TMessage ctor
	explicit SMsgFactProcessConfigurationReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactProcessConfigurationReply(const SMsgFactProcessConfigurationReply& theMsgR);

	// Destructor
	virtual ~SMsgFactProcessConfigurationReply(void);

	// Assignment
	SMsgFactProcessConfigurationReply& operator=(const SMsgFactProcessConfigurationReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	short GetStatus(void) const;
	const FACT_SERV_FIELD_REPLY_MAP& GetConfigFieldMap(void) const;

	virtual void SetStatus(short theStatus);
	virtual void SetConfigFieldMap(const FACT_SERV_FIELD_REPLY_MAP& theConfigFieldMap);

protected:
	short                     mStatus;
	FACT_SERV_FIELD_REPLY_MAP mConfigFieldMap;

};


// Inlines
inline TRawMsg* SMsgFactProcessConfigurationReply::Duplicate(void) const
{ return new SMsgFactProcessConfigurationReply(*this); }

inline short SMsgFactProcessConfigurationReply::GetStatus(void) const
{ return mStatus; }

inline const FACT_SERV_FIELD_REPLY_MAP& SMsgFactProcessConfigurationReply::GetConfigFieldMap(void) const
{ return mConfigFieldMap; }

inline void SMsgFactProcessConfigurationReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

inline void SMsgFactProcessConfigurationReply::SetConfigFieldMap(const FACT_SERV_FIELD_REPLY_MAP& theConfigFieldMap)
{ mConfigFieldMap = theConfigFieldMap; }

};  // Namespace WONMsg

#endif