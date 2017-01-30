#ifndef __SMsgUsrMultipleUserReply_H__
#define __SMsgUsrMultipleUserReply_H__

// SMsgUsrMultipleUserReply.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include <vector>
#include "../../base/SMsgUsrMultipleReplyBase.h"
#include "../../USER_SEARCH_SERVER_STRUCTURES.h"

#define SMsgUsrMultipleUserReplyTYPE WONMsg::EUserSearchFindUsersReply

namespace WONMsg 
	{
	class SMsgUsrMultipleUserReply : public SMsgUsrMultipleReplyBase
		{
		public:

			// Default ctor

			SMsgUsrMultipleUserReply(const unsigned char &i_reply_sequence_number = 0,
													NUserSearchServer::UserArray *p_output_userarray_for_unpacking = NULL);

			// SmallMessage ctor

			explicit SMsgUsrMultipleUserReply(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrMultipleUserReply(const SMsgUsrMultipleUserReply& theMsgR);

			// Destructor

			~SMsgUsrMultipleUserReply(void);

			// Assignment

			SMsgUsrMultipleUserReply& operator=(SMsgUsrMultipleUserReply& theMsgR);

			// Reply message specific data

			const unsigned long get_number_of_users() const;

			const unsigned long &get_users_begin() const;

			unsigned long &get_users_begin();

			const unsigned long &get_users_end() const;

			unsigned long &get_users_end();

			virtual unsigned long get_packed_size_in_bytes() const;

			NUserSearchServer::UserArray *get_output_userarray_for_unpacking()
				{
				return m_p_output_userarray_for_unpacking;
				}

			void set_output_userarray_for_unpacking(NUserSearchServer::UserArray &user_array)
				{
				m_p_output_userarray_for_unpacking = &user_array;
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

			NUserSearchServer::UserArray *m_p_output_userarray_for_unpacking;			// A designated pointer
																	// to an array of users that
																	// this message can unpack
																	// its users to.
		};

	// Inlines

	inline unsigned long SMsgUsrMultipleUserReply::get_service_type() const
		{
		return USER_SEARCH_SERVER_TYPE;
		}

	inline unsigned long SMsgUsrMultipleUserReply::get_message_type() const
		{
		return SMsgUsrMultipleUserReplyTYPE;
		}

	inline const unsigned long SMsgUsrMultipleUserReply::get_number_of_users() const
		{ 
		return m_user_objects_end - m_user_objects_begin;
		}

	inline const unsigned long &SMsgUsrMultipleUserReply::get_users_begin() const
		{
		return m_user_objects_begin;
		}

	inline unsigned long &SMsgUsrMultipleUserReply::get_users_begin()
		{
		return m_user_objects_begin;
		}

	inline const unsigned long &SMsgUsrMultipleUserReply::get_users_end() const
		{
		return m_user_objects_end;
		}

	inline unsigned long &SMsgUsrMultipleUserReply::get_users_end()
		{
		return m_user_objects_end;
		}

	inline TRawMsg*	SMsgUsrMultipleUserReply::Duplicate() const
		{ 
		return new SMsgUsrMultipleUserReply(*this); 
		}

	};  // Namespace WONMsg

#endif