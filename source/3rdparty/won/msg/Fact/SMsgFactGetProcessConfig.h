#if !defined(SMsgFactGetProcessConfiguration_H)
#define SMsgFactGetProcessConfiguration_H

// TMsgFactGetProcessConfiguration.h

// Message that is used to get a process configuration from the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

class SMsgFactGetProcessConfiguration : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetProcessConfiguration(void);

	// TMessage ctor
	explicit SMsgFactGetProcessConfiguration(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetProcessConfiguration(const SMsgFactGetProcessConfiguration& theMsgR);

	// Destructor
	virtual ~SMsgFactGetProcessConfiguration(void);

	// Assignment
	SMsgFactGetProcessConfiguration& operator=(const SMsgFactGetProcessConfiguration& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	const std::string& GetConfigName(void) const;
	virtual void SetConfigName(const std::string& theConfigName);

protected:
	std::string         mConfigName;

};


// Inlines
inline TRawMsg* SMsgFactGetProcessConfiguration::Duplicate(void) const
{ return new SMsgFactGetProcessConfiguration(*this); }

inline const std::string& SMsgFactGetProcessConfiguration::GetConfigName(void) const
{ return mConfigName; }

inline void SMsgFactGetProcessConfiguration::SetConfigName(const std::string& theConfigName)
{ mConfigName = theConfigName; }


};  // Namespace WONMsg

#endif