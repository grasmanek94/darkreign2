#ifndef __SMsgUsrGetPolledUserListReplyEx_H__
#define __SMsgUsrGetPolledUserListReplyEx_H__

// SMsgUsrGetPolledUserListReplyEx.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include <vector>
#include "../base/SMsgUsrMultipleReplyBase.h"
#include "../USER_SEARCH_SERVER_STRUCTURES.h"

#define GETPOLLEDUSERLISTREPLYENTEDEDTYPE WONMsg::EUserSearchGetPolledUserListReply

namespace WONMsg 
	{
	class SMsgUsrGetPolledUserListReplyEx : public SMsgUsrMultipleReplyBase
		{
		public:

			// Default ctor

			SMsgUsrGetPolledUserListReplyEx(const unsigned long &service_type,
										    const unsigned long &i_data_source_id,
										    const unsigned char &i_reply_sequence_number = 0,
										    const ServerStatus &server_status = WONMsg::StatusCommon_Success,
										    const ServerStatus &data_source_specific_status = WONMsg::StatusCommon_Success);

			// SmallMessage ctor

			explicit SMsgUsrGetPolledUserListReplyEx(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrGetPolledUserListReplyEx(const SMsgUsrGetPolledUserListReplyEx& theMsgR);

			// Destructor

			~SMsgUsrGetPolledUserListReplyEx(void);

			// Assignment

			SMsgUsrGetPolledUserListReplyEx& operator=(SMsgUsrGetPolledUserListReplyEx& theMsgR);

			// Reply message specific data

			const unsigned long get_number_of_users() const;

			virtual unsigned long get_packed_size_in_bytes() const;

			const NUserSearchServer::UserArray &get_users() const
				{
				return m_a_users;
				}

			NUserSearchServer::UserArray &get_users()
				{
				return m_a_users;
				}

			const unsigned long &get_data_source_id() const
				{
				return m_i_data_source_id;
				}

			unsigned long &get_data_source_id()
				{
				return m_i_data_source_id;
				}

			const ServerStatus &get_data_source_specific_status() const
				{
				return m_data_source_specific_status;
				}

			ServerStatus &get_data_source_specific_status()
				{
				return m_data_source_specific_status;
				}

		protected:

			virtual unsigned long get_service_type() const;

			virtual unsigned long get_message_type() const;

			virtual void pack_multiple_message_specific_data();
			
			virtual void unpack_multiple_message_specific_data();

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate() const;

		private:

			NUserSearchServer::UserArray m_a_users;									// The users in this message

			unsigned long m_i_data_source_id;						// The id of the data source
																	// for this message.

			ServerStatus m_data_source_specific_status;				// Data Source specific status
		};

	// Inlines

	inline unsigned long SMsgUsrGetPolledUserListReplyEx::get_service_type() const
		{
		return USER_SEARCH_SERVER_TYPE;
		}

	inline unsigned long SMsgUsrGetPolledUserListReplyEx::get_message_type() const
		{
		return GETPOLLEDUSERLISTREPLYENTEDEDTYPE;
		}

	inline const unsigned long SMsgUsrGetPolledUserListReplyEx::get_number_of_users() const
		{ 
		return m_a_users.size();
		}

	inline TRawMsg*	SMsgUsrGetPolledUserListReplyEx::Duplicate() const
		{ 
		return new SMsgUsrGetPolledUserListReplyEx(*this); 
		}

	};  // Namespace WONMsg

#endif