#ifndef _TITAN_CHAT_SERVER_TYPES_H_
#define _TITAN_CHAT_SERVER_TYPES_H_

#include "common/won.h"
#include <set>

namespace WONChat {

typedef unsigned long UserID;
typedef short UserDataType;
typedef short ChatDataType;
typedef std::set<UserID> UserIDList;

enum PredefinedUserIDConstants
{
	AllUsers	= 1L,
	NoUser		= 2L
};

enum PredefinedUserDataConstants
{
	CHAT_ALLUSERDATA = -1
};

enum PredefinedChatDataConstants
{
	CHAT_ERROR			= -1,
	CHAT_ASCII			= -2,
	CHAT_ASCII_EMOTE	= -3,
	CHAT_UNICODE		= -4,
	CHAT_UNICODE_EMOTE	= -5
};

class ChatData
{
public:
	ChatDataType	type;
	unsigned short	size;
	const void*		data;
	// chatData will only be valid as long as the message its from is valid!
	// When creating a new message, this must remain valid until the message is Pack()ed!
};


};

#endif
