#ifndef __SMsgUsrFindUserByCommunity_H__
#define __SMsgUsrFindUserByCommunity_H__

// SMsgUsrFindUserByCommunity.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include "../../base/SMsgUsrFindBase.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "msg/SServiceTypes.h"

#define SMsgUsrFindUserByCommunityTYPE WONMsg::EUserSearchFindUsersByName

namespace WONMsg 
	{
	class SMsgUsrFindUserByCommunity : public SMsgUsrFindBase
		{
		public:

			// Default ctor

			SMsgUsrFindUserByCommunity(const UserFindMatchMode &comparison_type = USFMM_EXACT,
									   const int &i_query_flags = 0,
									   unsigned short i_max_users_to_find = 0,
									   NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindUserByCommunity(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindUserByCommunity(const SMsgUsrFindUserByCommunity& source_message);

			// Destructor

			~SMsgUsrFindUserByCommunity(void);

			// Assignment

			SMsgUsrFindUserByCommunity& operator=(const SMsgUsrFindUserByCommunity& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Accessors for the private data

			const wstring &get_community_name_to_search_for() const
				{
				return m_s_community_name_to_search_for;
				}

			wstring &get_community_name_to_search_for()
				{
				return m_s_community_name_to_search_for;
				}

			const unsigned long &get_community_id_to_search_for() const
				{
				return m_i_community_id;
				}

			unsigned long &get_community_id_to_search_for()
				{
				return m_i_community_id;
				}

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
				return SMsgUsrFindUserByCommunityTYPE;
				}

		private:
			
			unsigned long m_i_community_id;			   // The community id of the
													   // community to find users in

			wstring m_s_community_name_to_search_for;  // List of user names to
													   // search on

			UserFindMatchMode m_comparison_type;	   // The comparison type
													   // for the name(s)
		};

	// Inlines

	inline TRawMsg* SMsgUsrFindUserByCommunity::Duplicate(void) const
		{ 
		return new SMsgUsrFindUserByCommunity(*this); 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindUserByCommunity_H__