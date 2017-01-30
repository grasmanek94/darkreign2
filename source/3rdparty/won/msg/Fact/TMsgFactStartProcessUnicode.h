#if !defined(TMsgFactStartProcessUnicode_H)
#define TMsgFactStartProcessUnicode_H

// TMsgFactStartProcessUnicode.h

// Message that is used to start a process via the Factory Server


#include "TMsgFactStartProcessBase.h"
#include <set>


namespace WONMsg {

	class TMsgFactStartProcessUnicode : public TMsgFactStartProcessBase {

public:
	// Default ctor
	TMsgFactStartProcessUnicode(void);

	// TMessage ctor
	explicit TMsgFactStartProcessUnicode(const TMessage& theMsgR);

	// Copy ctor
	TMsgFactStartProcessUnicode(const TMsgFactStartProcessUnicode& theMsgR);

	// Destructor
	virtual ~TMsgFactStartProcessUnicode(void);

	// Assignment
	TMsgFactStartProcessUnicode& operator=(const TMsgFactStartProcessUnicode& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

protected:
	virtual void PackCommandLine();
	virtual void UnpackCommandLine();
};


// Inlines
inline TRawMsg* TMsgFactStartProcessUnicode::Duplicate(void) const
{ return new TMsgFactStartProcessUnicode(*this); }

};  // Namespace WONMsg

#endif