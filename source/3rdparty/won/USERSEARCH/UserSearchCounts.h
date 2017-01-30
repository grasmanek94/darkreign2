#ifndef PRIZECENTRAL_H
#define PRIZECENTRAL_H

#include "Socket/IPSocket.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Authentication/Identity.h"
#include "msg/DB/SMsgDBRegisterUser.h"
#include "msg/ServerStatus.h"

namespace WONAPI 
	{
	struct UserSearchServerCountResult 
		{
		unsigned long m_result;
		WONMsg::ServerStatus m_status;

		UserSearchServerCountResult(const unsigned long &result,
									const WONMsg::ServerStatus status) : m_result(result),
																		 m_status(status)
			{
			}
		};

	// fail-over address list

	UserSearchServerCountResult UserSearchGetNumberOfUsers(Identity* identity, 
														   const IPSocket::Address* user_search_servers, 
														   unsigned int number_of_user_search_servers_in_array,
								 						   wstring requested_category,
														   long timeout, 
														   bool asynchronous, 
														   const CompletionContainer<const UserSearchServerCountResult&>& result_object);
	}; // namespace WONMsg;

#endif // PRIZECENTRAL_H
