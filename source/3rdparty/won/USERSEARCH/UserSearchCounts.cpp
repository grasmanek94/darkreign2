#include "Authentication/AuthSocket.h"
#include "msg/user/client/requests/SMsgUsrGetNumUsers.h"
#include "msg/user/client/replies/SMsgUsrGetNumUsersReply.h"
#include "msg/SServiceTypes.h"
#include "Errors.h"
#include "UserSearchCounts.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;

class UserSearchCounts 
	{
	public:

	TCPSocket m_tcp_socket;
	AuthSocket m_authentication_socket;
	IPSocket::Address* m_user_search_server_list;
	unsigned short m_number_of_user_search_servers;
	unsigned short m_current_server;
	CompletionContainer<const UserSearchServerCountResult &> result_object;

	SMsgUsrGetNumUsers m_request_message;
	SMsgUsrGetNumUsersReply m_response_message;

	long timeout;
	WONCommon::Event m_done_event;
	bool m_auto_delete;

	UserSearchCounts() : m_authentication_socket(m_tcp_socket, 
												 4)
		{ 
		};

	void Done()
		{
		if (m_user_search_server_list)
			{
			delete[] m_user_search_server_list;
			}

		result_object.Complete(UserSearchServerCountResult(m_response_message.get_number_of_users(),
														   m_response_message.GetStatus()));

		if (m_auto_delete)
			{
			delete this;
			}

		else
			{
			m_done_event.Set();
			}
		}
	};


static void on_done_request(const AuthSocket::AuthRecvBaseMsgResult& result, 
							UserSearchCounts* p_user_search_count_object);

// This function sends the actual request to the user search server

static void send_request_to_user_search_server(UserSearchCounts* p_user_search_count_object)
	{
	p_user_search_count_object->m_authentication_socket.Close(0);
	p_user_search_count_object->m_authentication_socket.ReleaseSession();
	p_user_search_count_object->m_tcp_socket.SetRemoteAddress(p_user_search_count_object->m_user_search_server_list[p_user_search_count_object->m_current_server]);

	p_user_search_count_object->m_authentication_socket.SendBaseMsgRequestEx(p_user_search_count_object->m_request_message, 
																			 0, 
																			 true, 
																			 true, 
																			 2000, 
																			 p_user_search_count_object->timeout, 
																			 true, 
																			 true, 
																			 on_done_request, 
																			 p_user_search_count_object);
	}

static void try_to_connect_to_next_user_search_server(UserSearchCounts* p_user_search_count_object)
	{
	if (++(p_user_search_count_object->m_current_server) == p_user_search_count_object->m_number_of_user_search_servers)
		{
		p_user_search_count_object->Done();
		}

	else
		{
		send_request_to_user_search_server(p_user_search_count_object);
		}
	}


static void on_done_request(const AuthSocket::AuthRecvBaseMsgResult& result, 
							UserSearchCounts* p_user_search_count_object)
	{
	if (result.msg)
		{
		auto_ptr<BaseMessage> autoDelReply(result.msg);

		p_user_search_count_object->m_response_message.SetStatus(StatusCommon_Failure);

		if (result.msg->GetServiceType() == SmallUserSearchServer && 
			result.msg->GetMessageType() == EUserSearchGetNumUsersReply)
			{
			try
				{
				p_user_search_count_object->m_response_message = SMsgUsrGetNumUsersReply(*((SmallMessage*)(result.msg)));

				p_user_search_count_object->m_response_message.Unpack();

				if (p_user_search_count_object->m_response_message.GetStatus() == StatusCommon_Success)
					{
					p_user_search_count_object->Done();

					return;
					}
				}

			catch (...)
				{
				// try next server
				}
			}
		}

	try_to_connect_to_next_user_search_server(p_user_search_count_object);
	}

UserSearchServerCountResult WONAPI::UserSearchGetNumberOfUsers(Identity* identity, 
															   const IPSocket::Address* user_search_servers, 
															   unsigned int number_of_user_search_servers_in_array,
								 							   wstring requested_category,
															   long timeout, 
															   bool asynchronous, 
															   const CompletionContainer<const UserSearchServerCountResult&>& result_object)
	{
	UserSearchServerCountResult result(0,
									   StatusCommon_Failure);

	if (number_of_user_search_servers_in_array > 0)
		{
		UserSearchCounts *p_user_search_server_query_object = new UserSearchCounts;

		if (p_user_search_server_query_object)
			{
			auto_ptr<UserSearchCounts> auto_delete_user_search_server_query_object(p_user_search_server_query_object);

			p_user_search_server_query_object->m_user_search_server_list = new IPSocket::Address[number_of_user_search_servers_in_array];

			if (p_user_search_server_query_object->m_user_search_server_list)
				{
				for (int i = 0; i < number_of_user_search_servers_in_array; i++)
					{
					p_user_search_server_query_object->m_user_search_server_list[i] = user_search_servers[i];
					}

				p_user_search_server_query_object->m_authentication_socket.SetIdentity(identity);
				p_user_search_server_query_object->m_auto_delete = asynchronous;
				p_user_search_server_query_object->result_object = result_object;
				p_user_search_server_query_object->timeout = timeout;
				p_user_search_server_query_object->m_current_server = 0;
				p_user_search_server_query_object->m_number_of_user_search_servers = number_of_user_search_servers_in_array;
				p_user_search_server_query_object->m_request_message.get_category_name() = requested_category;

				auto_delete_user_search_server_query_object.release();
				send_request_to_user_search_server(p_user_search_server_query_object);

				if (!asynchronous)
					{
					WSSocket::PumpUntil(p_user_search_server_query_object->m_done_event, 
										timeout);

					result.m_result = p_user_search_server_query_object->m_response_message.get_number_of_users();

					result.m_status = p_user_search_server_query_object->m_response_message.GetStatus();

					delete p_user_search_server_query_object;
					}

				return result;
				}
			}
		}

	// This is an error case, where no servers were provided

	result_object.Complete(result);

	return result;
	}
