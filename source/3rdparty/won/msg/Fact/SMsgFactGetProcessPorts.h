#if !defined(SMsgFactGetProcessPorts_H)
#define SMsgFactGetProcessPorts_H

// SMsgFactGetProcessPorts.h

// Message that is used to get the current ports in use by a process configuration from the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

class SMsgFactGetProcessPorts : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetProcessPorts(void);

	// TMessage ctor
	explicit SMsgFactGetProcessPorts(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetProcessPorts(const SMsgFactGetProcessPorts& theMsgR);

	// Destructor
	virtual ~SMsgFactGetProcessPorts(void);

	// Assignment
	SMsgFactGetProcessPorts& operator=(const SMsgFactGetProcessPorts& theMsgR);

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
inline TRawMsg* SMsgFactGetProcessPorts::Duplicate(void) const
{ return new SMsgFactGetProcessPorts(*this); }

inline const std::string& SMsgFactGetProcessPorts::GetConfigName(void) const
{ return mConfigName; }

inline void SMsgFactGetProcessPorts::SetConfigName(const std::string& theConfigName)
{ mConfigName = theConfigName; }

};  // Namespace WONMsg

#endif