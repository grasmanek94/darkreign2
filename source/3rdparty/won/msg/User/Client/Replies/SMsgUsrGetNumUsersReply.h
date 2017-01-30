#ifndef __SMsgUsrGetNumUsersReply_H__
#define __SMsgUsrGetNumUsersReply_H__

// SMsgUsrGetNumUsersReply.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "../../base/SMsgUsrReplyBase.h"

#define SMsgUsrGetNumUsersReplyTYPE WONMsg::EUserSearchGetNumUsersReply

namespace WONMsg 
	{
	class SMsgUsrGetNumUsersReply : public SMsgUsrReplyBase
		{
		public:

			// Default ctor

			SMsgUsrGetNumUsersReply(const unsigned long &i_number_of_users = 0);

			// SmallMessage ctor

			explicit SMsgUsrGetNumUsersReply(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrGetNumUsersReply(const SMsgUsrGetNumUsersReply& theMsgR);

			// Destructor

			~SMsgUsrGetNumUsersReply(void);

			// Assignment

			SMsgUsrGetNumUsersReply& operator=(const SMsgUsrGetNumUsersReply& theMsgR);

			// Reply message specific data

			const unsigned long &get_number_of_users() const;

			unsigned long &get_number_of_users();

			virtual unsigned long get_packed_size_in_bytes() const
				{
				return (SMsgUsrReplyBase::get_packed_size_in_bytes() + sizeof(unsigned long));
				}

		protected:

			virtual void pack_message_specific_data();
			
			virtual void unpack_message_specific_data();

			virtual unsigned long get_service_type() const;

			virtual unsigned long get_message_type() const;

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate() const;

		private:

			unsigned long m_i_number_of_users;  // Number of users found
		};

	// Inlines

	inline const unsigned long &SMsgUsrGetNumUsersReply::get_number_of_users() const
		{ 
		return m_i_number_of_users; 
		}

	inline unsigned long &SMsgUsrGetNumUsersReply::get_number_of_users()
		{ 
		return m_i_number_of_users; 
		}

	inline unsigned long SMsgUsrGetNumUsersReply::get_service_type() const
		{
		return USER_SEARCH_SERVER_TYPE;
		}

	inline unsigned long SMsgUsrGetNumUsersReply::get_message_type() const
		{
		return SMsgUsrGetNumUsersReplyTYPE;
		}

	inline TRawMsg*	SMsgUsrGetNumUsersReply::Duplicate() const
		{ 
		return new SMsgUsrGetNumUsersReply(*this); 
		}

	};  // Namespace WONMsg

#endif