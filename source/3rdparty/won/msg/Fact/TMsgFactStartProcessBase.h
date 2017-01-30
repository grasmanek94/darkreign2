#if !defined(TMsgFactStartProcessBase_H)
#define TMsgFactStartProcessBase_H

// TMsgFactStartProcessBase.h

// Message that is used to start a process via the Factory Server


#include "msg/TMessage.h"
#include <set>

#include "AllMsgStartProcessBase.h"

namespace WONMsg {

class TMsgFactStartProcessBase : public TMessage, public AllMsgStartProcessBase 
{

public:
	// Default ctor
	TMsgFactStartProcessBase(void);

	// TMessage ctor
	explicit TMsgFactStartProcessBase(const TMessage& theMsgR);

	// Copy ctor
	TMsgFactStartProcessBase(const TMsgFactStartProcessBase& theMsgR);

	// Destructor
	virtual ~TMsgFactStartProcessBase(void);

	// Assignment
	TMsgFactStartProcessBase& operator=(const TMsgFactStartProcessBase& theMsgR);

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);


protected:
	virtual void PackCommandLine() =0;
	virtual void UnpackCommandLine() =0;
};


};  // Namespace WONMsg

#endif