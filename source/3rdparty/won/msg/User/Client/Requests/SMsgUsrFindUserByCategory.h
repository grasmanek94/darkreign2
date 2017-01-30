#ifndef __SMsgUsrFindUserByCategory_H__
#define __SMsgUsrFindUserByCategory_H__

// SMsgUsrFindUserByCategory.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include "../../base/SMsgUsrFindBase.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "msg/SServiceTypes.h"
//#include "SmallMessageTypesUserSearch.h"


#define SMsgUsrFindUserByCategoryTYPE WONMsg::EUserSearchFindUsersByCategory

namespace WONMsg 
	{
	class SMsgUsrFindUserByCategory : public SMsgUsrFindBase
		{
		public:

			// Default ctor

			SMsgUsrFindUserByCategory(const wstring &category_to_find_users_from = L"",
													  const int &i_query_flags = 0,
													  unsigned short i_max_users_to_find = 0,
													  NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindUserByCategory(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindUserByCategory(const SMsgUsrFindUserByCategory& source_message);

			// Destructor

			~SMsgUsrFindUserByCategory(void);

			// Assignment

			SMsgUsrFindUserByCategory& operator=(const SMsgUsrFindUserByCategory& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Accessors for the private data

			const wstring &get_category_name(void) const;

			wstring &get_category_name(void);

			virtual void pack_message_specific_data();
			
			virtual void unpack_message_specific_data();

		protected:

			virtual unsigned long get_service_type() const
				{
				return USER_SEARCH_SERVER_TYPE;
				}

			virtual unsigned long get_message_type() const
				{
				return SMsgUsrFindUserByCategoryTYPE;
				}

		private:

			wstring m_s_category_name;  // List of directory paths/modes
		};

	// Inlines

	inline TRawMsg* SMsgUsrFindUserByCategory::Duplicate(void) const
		{ 
		return new SMsgUsrFindUserByCategory(*this); 
		}

	inline const wstring &SMsgUsrFindUserByCategory::get_category_name(void) const
		{ 
		return m_s_category_name; 
		}

	inline wstring &SMsgUsrFindUserByCategory::get_category_name(void)
		{ 
		return m_s_category_name; 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindUserByCategory_H__