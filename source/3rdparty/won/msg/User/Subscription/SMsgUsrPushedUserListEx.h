#ifndef __SMsgUsrPushedUserListEx_H__
#define __SMsgUsrPushedUserListEx_H__

// SMsgUsrPushedUserListEx.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include <vector>
#include "../base/SMsgUsrMultipleReplyBase.h"
#include "../USER_SEARCH_SERVER_STRUCTURES.h"

#define SMsgUsrPushedUserListExTYPE WONMsg::EUserSearchPushedUserList

namespace WONMsg 
	{
	class SMsgUsrPushedUserListEx : public SMsgUsrMultipleReplyBase
		{
		public:

			// Default ctor

			SMsgUsrPushedUserListEx(const unsigned long &i_data_source_id,
								    const unsigned long &i_issue_number,
								    const unsigned char &i_reply_sequence_number = 0,
									const ServerStatus &server_status = WONMsg::StatusCommon_Success,
								    const ServerStatus &data_source_specific_status = WONMsg::StatusCommon_Success);

			// SmallMessage ctor

			explicit SMsgUsrPushedUserListEx(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrPushedUserListEx(const SMsgUsrPushedUserListEx& theMsgR);

			// Destructor

			~SMsgUsrPushedUserListEx(void);

			// Assignment

			SMsgUsrPushedUserListEx& operator=(SMsgUsrPushedUserListEx& theMsgR);

			// Reply message specific data

			const unsigned long get_number_of_users() const;

			virtual unsigned long get_packed_size_in_bytes() const;

			const UserUpdateArray &get_users() const
				{
				return m_a_users;
				}

			UserUpdateArray &get_users()
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

			const unsigned long &get_issue_number() const
				{
				return m_i_issue_number;
				}

			unsigned long &get_issue_number()
				{
				return m_i_issue_number;
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

			UserUpdateArray m_a_users;									// The users in this message

			unsigned long m_i_data_source_id;						// The id of the data source
																	// for this message.

			unsigned long m_i_issue_number;							// The subscription issue number

			ServerStatus m_data_source_specific_status;				// Data Source specific status
		};

	// Inlines

	inline unsigned long SMsgUsrPushedUserListEx::get_service_type() const
		{
		return USER_SEARCH_SERVER_TYPE;
		}

	inline unsigned long SMsgUsrPushedUserListEx::get_message_type() const
		{
		return SMsgUsrPushedUserListExTYPE;
		}

	inline const unsigned long SMsgUsrPushedUserListEx::get_number_of_users() const
		{ 
		return m_a_users.size();
		}

	inline TRawMsg*	SMsgUsrPushedUserListEx::Duplicate() const
		{ 
		return new SMsgUsrPushedUserListEx(*this); 
		}

	};  // Namespace WONMsg

#endif