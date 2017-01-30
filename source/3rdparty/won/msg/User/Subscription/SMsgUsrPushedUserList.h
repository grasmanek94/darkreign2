#ifndef __SMsgUsrPushedUserList_H__
#define __SMsgUsrPushedUserList_H__

// SMsgUsrPushedUserList.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include <vector>
#include "../Base/SMsgUsrMultipleReplyBase.h"
#include "../USER_SEARCH_SERVER_STRUCTURES.h"

#define SMsgUsrPushedUserListTYPE WONMsg::EUserSearchPushedUserList

namespace WONMsg 
	{
	class SMsgUsrPushedUserList : public SMsgUsrMultipleReplyBase
		{
		public:

			// Default ctor

			SMsgUsrPushedUserList(const unsigned long &i_data_source_id,
							      const unsigned long &i_issue_number,
							      const unsigned char &i_reply_sequence_number = 0,
							      UserUpdateArray *p_output_userarray_for_unpacking = NULL,
								  const ServerStatus &server_status = WONMsg::StatusCommon_Success,
							      const ServerStatus &data_source_specific_status = WONMsg::StatusCommon_Success);

			// SmallMessage ctor

			explicit SMsgUsrPushedUserList(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrPushedUserList(const SMsgUsrPushedUserList& theMsgR);

			// Destructor

			~SMsgUsrPushedUserList(void);

			// Assignment

			SMsgUsrPushedUserList& operator=(SMsgUsrPushedUserList& theMsgR);

			// Reply message specific data

			const unsigned long get_number_of_users() const;

			const unsigned long &get_users_begin() const;

			unsigned long &get_users_begin();

			const unsigned long &get_users_end() const;

			unsigned long &get_users_end();

			virtual unsigned long get_packed_size_in_bytes() const;

			UserUpdateArray *get_output_userarray_for_unpacking()
				{
				return m_p_output_userarray_for_unpacking;
				}

			void set_output_userarray_for_unpacking(UserUpdateArray &user_array)
				{
				m_p_output_userarray_for_unpacking = &user_array;
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

			unsigned long m_user_objects_begin;						// An index into the
																	// master vector of users,
																	// describing the beginning
																	// part that this message is
																	// concerned with.

			unsigned long m_user_objects_end;						// An index into the
																	// master vector of users,
																	// describing the end
																	// part that this message is
																	// concerned with.

			UserUpdateArray *m_p_output_userarray_for_unpacking;	// A designated pointer
																	// to an array of users that
																	// this message can unpack
																	// its users to.

			unsigned long m_i_data_source_id;						// The id of the data source
																	// for this message.

			unsigned long m_i_issue_number;							// The subscription issue number

			ServerStatus m_data_source_specific_status;				// Data Source specific status
		};

	// Inlines

	inline unsigned long SMsgUsrPushedUserList::get_service_type() const
		{
		return USER_SEARCH_SERVER_TYPE;
		}

	inline unsigned long SMsgUsrPushedUserList::get_message_type() const
		{
		return SMsgUsrPushedUserListTYPE;
		}

	inline const unsigned long SMsgUsrPushedUserList::get_number_of_users() const
		{ 
		return m_user_objects_end - m_user_objects_begin;
		}

	inline const unsigned long &SMsgUsrPushedUserList::get_users_begin() const
		{
		return m_user_objects_begin;
		}

	inline unsigned long &SMsgUsrPushedUserList::get_users_begin()
		{
		return m_user_objects_begin;
		}

	inline const unsigned long &SMsgUsrPushedUserList::get_users_end() const
		{
		return m_user_objects_end;
		}

	inline unsigned long &SMsgUsrPushedUserList::get_users_end()
		{
		return m_user_objects_end;
		}

	inline TRawMsg*	SMsgUsrPushedUserList::Duplicate() const
		{ 
		return new SMsgUsrPushedUserList(*this); 
		}

	};  // Namespace WONMsg

#endif