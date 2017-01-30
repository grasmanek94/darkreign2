#ifndef __SMsgUsrFindBase_H__
#define __SMsgUsrFindBase_H__

// SMsgUsrFindBase.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "../USER_SEARCH_SERVER_STRUCTURES.h"

namespace WONMsg 
	{
	class SMsgUsrFindBase : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrFindBase(const unsigned long &service_type,
												const unsigned long &message_type,
												const int &i_query_flags = 0,
												unsigned short i_max_users_to_find = 0,
												NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindBase(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindBase(const SMsgUsrFindBase& source_message);

			// Destructor

			~SMsgUsrFindBase(void);

			// Assignment

			SMsgUsrFindBase& operator=(const SMsgUsrFindBase& source_message);

			// Accessors for the private data

			const int &get_query_flags(void) const;

			int &get_query_flags(void);

			const unsigned short &get_max_users_to_find() const
				{
				return m_i_max_users_to_find;
				}

			unsigned short &get_max_users_to_find()
				{
				return m_i_max_users_to_find;
				}

			const NUserSearchServer::UserActivityDataTypes &get_activity_data_types() const
				{
				return m_activity_data_types;
				}

			NUserSearchServer::UserActivityDataTypes &get_activity_data_types()
				{
				return m_activity_data_types;
				}

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

		protected:

			virtual void pack_message_specific_data() = 0;
			
			virtual void unpack_message_specific_data() = 0;

			virtual unsigned long get_service_type() const = 0;

			virtual unsigned long get_message_type() const = 0;

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate(void) const = 0;

		private:

			int										 m_i_query_flags;		 // The query flags
			unsigned short							 m_i_max_users_to_find;  // The maximum number of user to find
			NUserSearchServer::UserActivityDataTypes  m_activity_data_types;
		};

	// Inlines

	inline const int &SMsgUsrFindBase::get_query_flags(void) const
		{ 
		return m_i_query_flags; 
		}

	inline int &SMsgUsrFindBase::get_query_flags(void)
		{ 
		return m_i_query_flags; 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindBase_H__