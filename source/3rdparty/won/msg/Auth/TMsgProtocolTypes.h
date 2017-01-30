#ifndef _TMsgProtocolTypes_H_
#define _TMsgProtocolTypes_H_

//
// Titan Protocol types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONMsg
{
	enum MsgProtocolType
	{
		// Auth Server messages
		TProtMin        = 0,
        TProtNone       = 0,

		TAuth0Auth    = 1,  // unsecure AuthServer authentication
        TAuth0Peer    = 2,  // unsecure Peer authentication

        TAuth1Auth    = 10, // AuthServer login/authentication
        TAuth1Peer    = 11, // Peer authentication

		// Last Message type.  Don't use
		TProtMax
	};
};

#endif
