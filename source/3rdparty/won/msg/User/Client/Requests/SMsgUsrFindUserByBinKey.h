#ifndef __SMsgUsrFindUserByBinKey_H__
#define __SMsgUsrFindUserByBinKey_H__

// SMsgUsrFindUserByBinKey.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include "../../base/SMsgUsrFindBase.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "msg/SServiceTypes.h"

#define SMsgUsrFindUserByBinKeyTYPE WONMsg::EUserSearchFindUsersByBinaryKey

namespace WONMsg 
	{
	class SMsgUsrFindUserByBinKey : public SMsgUsrFindBase
		{
		public:

			// Default ctor

			SMsgUsrFindUserByBinKey(const UserFindMatchMode &comparison_type = USFMM_EXACT,
								    const int &i_query_flags = 0,
								    unsigned short i_max_users_to_find = 0,
								    NUserSearchServer::UserActivityDataTypes *activity_data_types = NULL);

			// SmallMessage ctor

			explicit SMsgUsrFindUserByBinKey(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrFindUserByBinKey(const SMsgUsrFindUserByBinKey& source_message);

			// Destructor

			~SMsgUsrFindUserByBinKey(void);

			// Assignment

			SMsgUsrFindUserByBinKey& operator=(const SMsgUsrFindUserByBinKey& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Accessors for the private data

			const std::vector<WONCommon::RawBuffer> &get_keys_to_search_for() const;

			std::vector<WONCommon::RawBuffer> &get_keys_to_search_for();

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
				return SMsgUsrFindUserByBinKeyTYPE;
				}

		private:

			
			std::vector<WONCommon::RawBuffer> m_keys_to_search_for;  // List of keys to
													  			     // search for

			UserFindMatchMode m_comparison_type;				// The comparison type
																// for the name(s)
		};

	// Inlines

	inline TRawMsg* SMsgUsrFindUserByBinKey::Duplicate(void) const
		{ 
		return new SMsgUsrFindUserByBinKey(*this); 
		}

	inline std::vector<WONCommon::RawBuffer> &SMsgUsrFindUserByBinKey::get_keys_to_search_for()
		{ 
		return m_keys_to_search_for; 
		}

	inline const std::vector<WONCommon::RawBuffer> &SMsgUsrFindUserByBinKey::get_keys_to_search_for() const
		{ 
		return m_keys_to_search_for; 
		}

	};  // Namespace WONMsg

#endif // __SMsgUsrFindUserByBinKey_H__