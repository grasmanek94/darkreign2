#if !defined(TMsgTypesFact_H)
#define TMsgTypesFact_H

//
// Titan Factory Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum MsgTypeFact
	{
		// First message type.  Don't use
		FactMsgMin = 0,

		// Factory Server Messages
		FactStatusReply           = FactMsgMin+1,
		FactStartProcess          = FactMsgMin+2,
		FactStartProcessUnicode   = FactMsgMin+8,


		// Last Message type.  Don't use
		FactMsgMax
	};

};

#endif