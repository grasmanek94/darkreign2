#ifndef __SMsgUsrFindUserByWONId_H__
#define __SMsgUsrFindUserByWONId_H__

// SMsgUsrFindUserByWONId.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include "../../base/SMsgUsrFindBase.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "msg/SServiceTypes.h"

#define SMsgUsrFindUserByWONIdTYPE WONMsg::EUserSearchFindUsersByWONId

namespace WONMsg 
	{
	class SMsgUsrFindUserByWONId : public SMsgUsrFindBase
		{
		public:

			// Default ctor

			SMsgUsrFindUserByWONId(const int &i_query_flags = 0,
												   unsigned short i_max_users_to_find = 0,
												   NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindUserByWONId(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindUserByWONId(const SMsgUsrFindUserByWONId& source_message);

			// Destructor

			~SMsgUsrFindUserByWONId(void);

			// Assignment

			SMsgUsrFindUserByWONId& operator=(const SMsgUsrFindUserByWONId& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Accessors for the private data

			const std::vector<unsigned long> &get_won_ids_to_search_for() const;

			std::vector<unsigned long> &get_won_ids_to_search_for();

			virtual void pack_message_specific_data();
			
			virtual void unpack_message_specific_data();

		protected:

			virtual unsigned long get_service_type() const
				{
				return USER_SEARCH_SERVER_TYPE;
				}

			virtual unsigned long get_message_type() const
				{
				return SMsgUsrFindUserByWONIdTYPE;
				}

		private:

			
			std::vector<unsigned long> m_won_ids_to_search_for;  // List of WON IDs to
																 // search on
		};

	// Inlines

	inline TRawMsg* SMsgUsrFindUserByWONId::Duplicate(void) const
		{ 
		return new SMsgUsrFindUserByWONId(*this); 
		}

	inline std::vector<unsigned long> &SMsgUsrFindUserByWONId::get_won_ids_to_search_for()
		{ 
		return m_won_ids_to_search_for; 
		}

	inline const std::vector<unsigned long> &SMsgUsrFindUserByWONId::get_won_ids_to_search_for() const
		{ 
		return m_won_ids_to_search_for; 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindUserByWONId_H__