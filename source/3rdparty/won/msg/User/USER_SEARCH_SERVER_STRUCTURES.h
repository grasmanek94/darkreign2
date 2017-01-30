#ifndef __USER_SEARCH_SERVER_STRUCTURES_H__
#define __USER_SEARCH_SERVER_STRUCTURES_H__

#include "UserFlags.h"
#include "msg/TMessage.h"
#include <vector>

// User Search Server

// Defines the data structures used by the user search server.

// All users are stored using a multiset (to allow for the same
// user logging in more than once) in a master "list" primarily 
// indexed by WONUserId (a long integer type), and secondarily 
// indexed by WON user name. All data needed by the user search 
// server for a given user is stored exactly once for each server 
// "login" instance of that user, in this multiset. Searches for 
// users based on WONUserId will be logarithmic. For non-WONUserId 
// user searches, the multimaps described below can be used to 
// preserve order-logarithmic searches in most cases. Users can be 
// in exactly one category. Each user has also has a pointer to the 
// map containing that user in the user's current category, as well 
// as an iterator to the user entry, within the category user map. 
// This is necessary to allow for logarithmic time deletion of users
// from the user search server.

// In order to facilitate logarithmic searches on keys other
// than the WONUserId, several maps are also defined, mapping
// from non-WONUserId data to the user data objects stored
// in the master user multiset.

// User objects support Activity DataObjects, which allows servers to 
// extend activity specific data in any manner.  DataObjects contain 
// a binary type block with an associated binary data block.

// Categories are stored using node objects. Categories that do not 
// contain users are non-leaf categories that generally do have children.
// Leaf categories have no child caterogies, but have a map to contained
// users.

// All category nodes are stored in a master category set keyed on
// category name. Each node has an iterator to its parent category
// node. Non-leaf nodes each have a map keyed on category name that
// has pointers to child nodes as data. Leaf nodes each have a map
// primarily keyed on WON user name. 

// Since STL sets are being used for storage, only const references can
// be fetched.  As a result, the maps are storing pointers to users rather
// than the users themselves.  This allows the nodes to be updated when
// fetched from a map.

// *IMPORTANT* Never update key fields of a user in a map.  This will
// corrupt the map.


///////////////////////////////////////////////////
// The following are valid data source typenames //
												 //
#define TITANROUTINGSERVER "TitanRoutingServer"	 //
												 //
///////////////////////////////////////////////////

namespace NUserSearchServer
	{
	typedef unsigned char BYTE;

	// The collection of activity data object types requested in a client message

	class UserActivityDataTypes
		{
		public:

			UserActivityDataTypes();

			UserActivityDataTypes(const UserActivityDataTypes &source_object);

			void pack_data(WONMsg::SmallMessage &reply_message) const;
			void unpack_data(const WONMsg::SmallMessage &reply_message);

			virtual unsigned long get_packed_size_in_bytes() const;

			const unsigned short &get_number_of_data_object_types() const
				{
				return m_i_number_of_data_object_types;
				}
			
			const std::vector<WONCommon::RawBuffer> &get_data_object_types() const
				{
				return m_data_object_types;
				}

		protected:

			unsigned short m_i_number_of_data_object_types;
				// The number of activity data object types
				// in this instance

			std::vector<WONCommon::RawBuffer> m_data_object_types;
				// The data types requested
		};

	// The structure for an activity data object

	class UserActivityDataObject
		{
		public:

			UserActivityDataObject();

			void pack_data(WONMsg::SmallMessage &reply_message) const;
			void unpack_data(const WONMsg::SmallMessage &reply_message);

			virtual unsigned long get_packed_size_in_bytes() const;

			WONCommon::RawBuffer m_data_type;
				// The binary field identifying the
				// data type for this data object

			WONCommon::RawBuffer m_data;
				// The binary field for the actual
				// data
		};

	// The collection of activity data objects associated
	// with a given user

	class UserActivityData
		{
		public:

			UserActivityData();

			void pack_data(WONMsg::SmallMessage &reply_message) const;
			void unpack_data(const WONMsg::SmallMessage &reply_message);

			virtual unsigned long get_packed_size_in_bytes() const;

			unsigned short m_i_number_of_data_objects;
				// The number of activity data objects
				// in this instance

			std::vector<UserActivityDataObject> m_data_objects;
				// The actual data objects 
		};

	// The data associated with a user

	class User
		{
		public:

			User();

			void pack_data(WONMsg::SmallMessage &reply_message) const;
			void unpack_data(const WONMsg::SmallMessage &reply_message);

			virtual unsigned long get_packed_size_in_bytes() const;

			wstring m_s_username;
				// The user's username

			unsigned long m_i_won_user_id;
				// The user's WON User ID

			unsigned long m_i_community_id;
				// The user's community ID

			WONMsg::EStatusType m_i_status;
				// The user's current status

			WONCommon::RawBuffer m_location_binary;
				// A binary field, generally
				// for locational data

			WONCommon::RawBuffer m_key_binary;
				// A binary field, generally
				// for search key data

			UserActivityData m_activity_data;
				// The activity data associated
				// with this user
		};

	typedef std::vector<User> UserArray;
	};

namespace WONMsg 
	{
	enum Purpose
		{
		EPurposeAdd,
		EPurposeDelete,
		EPurposeUpdate,

		EPurposeTotalTypes
		};

	class UserUpdateObject
		{
		public:

			UserUpdateObject(const Purpose &update_purpose = EPurposeAdd) :m_update_purpose(update_purpose) {};
		
			NUserSearchServer::User m_user;

			Purpose m_update_purpose;
		};

	typedef std::vector<UserUpdateObject> UserUpdateArray;
	}

#endif