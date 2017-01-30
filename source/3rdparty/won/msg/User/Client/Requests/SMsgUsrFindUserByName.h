#ifndef __SMsgUsrFindUserByName_H__
#define __SMsgUsrFindUserByName_H__

// SMsgUsrFindUserByName.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include "../../base/SMsgUsrFindBase.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "msg/SServiceTypes.h"

#define SMsgUsrFindUserByNameTYPE WONMsg::EUserSearchFindUsersByName

namespace WONMsg 
	{
	class SMsgUsrFindUserByName : public SMsgUsrFindBase
		{
		public:

			// Default ctor

			SMsgUsrFindUserByName(const UserFindMatchMode &comparison_type = USFMM_EXACT,
												  const int &i_query_flags = 0,
												  unsigned short i_max_users_to_find = 0,
												  NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindUserByName(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindUserByName(const SMsgUsrFindUserByName& source_message);

			// Destructor

			~SMsgUsrFindUserByName(void);

			// Assignment

			SMsgUsrFindUserByName& operator=(const SMsgUsrFindUserByName& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Accessors for the private data

			const std::vector<wstring> &get_user_names_to_search_for() const;

			std::vector<wstring> &get_user_names_to_search_for();

			const UserFindMatchMode &get_comparison_type() const
				{
				return m_comparison_type;
				}

			UserFindMatchMode &get_comparison_type()
				{
				return m_comparison_type;
				}

			virtual void pack_message_specific_data();
			
			virtual void unpack_message_specific_data();

		protected:

			virtual unsigned long get_service_type() const
				{
				return USER_SEARCH_SERVER_TYPE;
				}

			virtual unsigned long get_message_type() const
				{
				return SMsgUsrFindUserByNameTYPE;
				}

		private:

			
			std::vector<wstring> m_s_user_names_to_search_for;  // List of user names to
																// search on

			UserFindMatchMode m_comparison_type;				// The comparison type
																// for the name(s)
		};

	// Inlines

	inline TRawMsg* SMsgUsrFindUserByName::Duplicate(void) const
		{ 
		return new SMsgUsrFindUserByName(*this); 
		}

	inline std::vector<wstring> &SMsgUsrFindUserByName::get_user_names_to_search_for()
		{ 
		return m_s_user_names_to_search_for; 
		}

	inline const std::vector<wstring> &SMsgUsrFindUserByName::get_user_names_to_search_for() const
		{ 
		return m_s_user_names_to_search_for; 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindUserByName_H__