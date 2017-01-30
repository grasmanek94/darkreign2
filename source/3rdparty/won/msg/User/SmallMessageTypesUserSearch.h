#ifndef __SmallMessageTypesUserSearch_H__
#define __SmallMessageTypesUserSearch_H__

//
// Titan User Search Server message types
// These values must be fixed in order to communicate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

#define USER_SEARCH_SERVER_TYPE WONMsg::SmallUserSearchServer

namespace WONMsg
{
	enum MsgTypeUserSearch
	{
		// First message type.  Don't use
		EUserSearchMin,

		// User Search Server Messages

//		EUserSearchStatusReply			= 1,

		// Requests to the Server

		EUserSearchFindUsersByName,
		EUserSearchFindUsersByWONId,
		EUserSearchFindUsersByCategory,
		EUserSearchFindUsersByCommunity,
		EUserSearchFindUsersByLocation,
		EUserSearchFindUsersByBinaryKey,
		EUserSearchFindUsers,
		EUserSearchGetNumUsers,

		// Responses from the Server

		EUserSearchFindUsersReply,
		EUserSearchGetNumUsersReply,

		// Polled User Data Source Messages

		EUserSearchGetPolledUserList,
		EUserSearchGetPolledUserListReply,

		// Subscribed User Data Source Messages

		EUserSearchSubscribeToPushedUserList,
		EUserSearchPushedUserList,

		// Self-Registered Data Sources

		EUserSearchRegisterDataSource,
		EUserSearchRegisterDataSourceReply,
		EUserSearchRemoveDataSource,

		// Reconnects

		EUserSearchReconnectDataSource,
		EUserSearchReconnectSelfRegisteredDataSource,

		// Last Message type.  Don't use

		EUserSearchMessageMax
	};

};

#endif // __SmallMessageTypesUserSearch_H__