#ifndef _MMsgCommNoOp_H
#define _MMsgCommNoOp_H

// MMsgCommNoOp.h

// Common Message NoOp Message class.  Can be sent to a Titan server
// without eliciting any response, error or otherwise.  Can be useful
// as a "keep-alive" message.

#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgCommNoOp : public MiniMessage
{
public:
	// Default ctor
	MMsgCommNoOp(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit MMsgCommNoOp(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommNoOp(const MMsgCommNoOp& theMsgR);

	// Destructor
	~MMsgCommNoOp(void);

	// Assignment
	MMsgCommNoOp& operator=(const MMsgCommNoOp& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);
};


// Inlines
inline TRawMsg*
MMsgCommNoOp::Duplicate(void) const
{ return new MMsgCommNoOp(*this); }

};  // Namespace WONMsg

#endif