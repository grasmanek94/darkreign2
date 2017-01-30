// DirServerNodes

// Defines the nodes stored by the DirectoryServer.  Currently,
// Service and Directory nodes are supported.  A ServiceNode
// contains information about a registered service.  A DirectoryNode
// contains information about a directory as well as a set of services
// and a set of subdirectories.

// The data structures for storing nodes in the Directory Server are
// optimized for find, add, and remove operations.  Change operations
// are slow when key fields are altered as they must do a remove followed
// by an add.

// All nodes support DataObjects, which allows clients to extend node
// attributes in any manner.  DataObjects are a binary type block with an
// associated binary data block.

// Since STL sets are being used for storage, only const references can
// be fetched.  As a result, the sets are storing pointers to nodes rather
// than the nodes themselves.  This allows the nodes to be updated when
// fetched from a set.
// *IMPORTANT* Never update key fields of a node in a set.  This will
// corrupt the set.

// DirectoryNodes are keyed by their path.  ServiceNodes are keyed by their
// ServiceKey which includes name and netAddress.


#include "common/won.h"
#include <set>
#include <time.h>
#include "common/OutputOperators.h"
#include "framework/PermissionMgr.h"
#include "USER_SEARCH_SERVER_STRUCTURES.h"
#include "msg/TMessage.h"


// Private namespace
namespace {
	using WONCommon::RawBuffer;
	using WONCommon::StringToWString;

	using WONMsg::SmallMessage;

	using NUserSearchServer::UserActivityDataObject;
	using NUserSearchServer::UserActivityData;
	using NUserSearchServer::User;
	using NUserSearchServer::UserActivityDataTypes;
};

User::User()
	 :m_i_won_user_id(0),
	  m_i_community_id(0),
	  m_i_status(WONMsg::EStatusType(0))
	{
	}

unsigned long User::get_packed_size_in_bytes() const
	{
	unsigned long result = 0;

	// Add the size of the length of the username

	result += sizeof(unsigned short);

	// Add the size of the username (each character is a wchar, not always a 1 byte char)

	result += (sizeof(wchar) * m_s_username.size());

	// Add the size of the WON User ID

	result += sizeof(unsigned long);

	// Add the size of the community ID

	result += sizeof(unsigned long);

	// Add the size of the status

	result += sizeof(unsigned char);

	// Add the size of the size of the binary location field

	result += sizeof(unsigned long);

	// Add the size of the binary location field itself

	result += m_location_binary.size();

	// Add the size of the size of the binary key field

	result += sizeof(unsigned long);

	// Add the size of the binary key field itself

	result += m_key_binary.size();

	// Add the size of the user activity data objects

	result += m_activity_data.get_packed_size_in_bytes();

	return result;
	}

void User::pack_data(SmallMessage &reply_message) const
	{
	// Pack the username

	reply_message.Append_PW_STRING(m_s_username);

	// Pack the WON User ID

	reply_message.AppendLong(m_i_won_user_id);

	// Pack the community id

	reply_message.AppendLong(m_i_community_id);

	// Pack the status

	reply_message.AppendByte(m_i_status);

	// Append the size of the binary location field

	reply_message.AppendLong(m_location_binary.size());

	// Append the binary location field itself

	reply_message.AppendBytes(m_location_binary.size(), 
							  m_location_binary.data());

	// Append the size of the binary key field

	reply_message.AppendLong(m_key_binary.size());

	// Append the binary key field itself

	reply_message.AppendBytes(m_key_binary.size(), 
							  m_key_binary.data());

	// Append the activity data

	m_activity_data.pack_data(reply_message);
	}

void User::unpack_data(const SmallMessage &reply_message)
	{
	// Retrieve the username

	reply_message.ReadWString(m_s_username);

	// Retrieve the WON User ID

	m_i_won_user_id = reply_message.ReadLong();

	// Retrieve the community id

	m_i_community_id = reply_message.ReadLong();

	// Retrieve the status

	m_i_status = WONMsg::EStatusType(reply_message.ReadByte());

	// Retrieve the size of the binary location field

	unsigned long location_binary_size = reply_message.ReadLong();

	// Retrieve the binary location field itself

	m_location_binary.assign(reinterpret_cast<const unsigned char*>(reply_message.ReadBytes(location_binary_size)), 
							 location_binary_size);

	// Retrieve the size of the binary key field

	unsigned long key_binary_size = reply_message.ReadLong();

	// Retrieve the binary key field itself

	m_key_binary.assign(reinterpret_cast<const unsigned char*>(reply_message.ReadBytes(key_binary_size)), 
						key_binary_size);

	// Retrieve the activity data

	m_activity_data.unpack_data(reply_message);
	}

UserActivityData::UserActivityData()
				 :m_data_objects(),
				  m_i_number_of_data_objects(0)
	{
	};

unsigned long UserActivityData::get_packed_size_in_bytes() const
	{
	unsigned long result = 0;

	// Add the size of the number of data objects

	result += sizeof(unsigned short);

	// Cycle through the vector of data objects, and add the size of each data object

	std::vector<UserActivityDataObject>::const_iterator data_objects_iterator(m_data_objects.begin());

	for (;
	     data_objects_iterator != m_data_objects.end();
		 data_objects_iterator++)
		{
		result += data_objects_iterator->get_packed_size_in_bytes();
		}

	return result;
	}

void UserActivityData::pack_data(SmallMessage &reply_message) const
	{
	// Append the number of data objects

	reply_message.AppendShort(m_i_number_of_data_objects);

	// Cycle through the vector of data objects, and append each data object

	std::vector<UserActivityDataObject>::const_iterator data_objects_iterator(m_data_objects.begin());

	for (;
	     data_objects_iterator != m_data_objects.end();
		 data_objects_iterator++)
		{
		data_objects_iterator->pack_data(reply_message);
		}
	}

void UserActivityData::unpack_data(const SmallMessage &reply_message)
	{
	// Retrieve the number of data objects

	m_i_number_of_data_objects = reply_message.ReadShort();

	// Resize the vector to store the correct number of data objects

	m_data_objects.resize(m_i_number_of_data_objects);

	// Cycle through the vector of data objects, and retrieve each data object

	std::vector<UserActivityDataObject>::iterator data_objects_iterator(m_data_objects.begin());

	for (;
	     data_objects_iterator != m_data_objects.end();
		 data_objects_iterator++)
		{
		data_objects_iterator->unpack_data(reply_message);
		}
	}

UserActivityDataObject::UserActivityDataObject()
	{
	};

unsigned long UserActivityDataObject::get_packed_size_in_bytes() const
	{
	unsigned long result = 0;

	// Add the size of the length of the data type

	result += sizeof(unsigned long);

	// Add the size of the data type itself

	result += m_data_type.size();

	// Add the size of the length of the data

	result += sizeof(unsigned long);

	// Add the size of the data

	result += m_data.size();

	return result;
	}

void UserActivityDataObject::pack_data(SmallMessage &reply_message) const
	{
	// Append the length of the data type

	reply_message.AppendLong(m_data_type.size());

	// Append the data type itself

	reply_message.AppendBytes(m_data_type.size(), 
							  m_data_type.data());

	// Append the length of the data

	reply_message.AppendLong(m_data.size());

	// Append the data itself

	reply_message.AppendBytes(m_data.size(), 
							  m_data.data());
	}

void UserActivityDataObject::unpack_data(const SmallMessage &reply_message)
	{
	// Retrieve the length of the data type

	unsigned long data_type_size = reply_message.ReadLong();

	// Retrieve the data type itself

	m_data_type.assign(reinterpret_cast<const unsigned char*>(reply_message.ReadBytes(data_type_size)), 
					   data_type_size);

	// Retrieve the length of the data

	unsigned long data_size = reply_message.ReadLong();

	// Retrieve the data itself

	m_data.assign(reinterpret_cast<const unsigned char*>(reply_message.ReadBytes(data_size)), 
				  data_size);
	}

UserActivityDataTypes::UserActivityDataTypes()
					  :m_data_object_types(),
					   m_i_number_of_data_object_types(0)
	{
	};

UserActivityDataTypes::UserActivityDataTypes(const UserActivityDataTypes &source_object)
					  :m_data_object_types(source_object.get_data_object_types()),
					   m_i_number_of_data_object_types(source_object.get_number_of_data_object_types())
	{
	}

unsigned long UserActivityDataTypes::get_packed_size_in_bytes() const
	{
	unsigned long result = 0;

	// Append the size of the number of activity data object types

	result += sizeof(unsigned short);

	// Cycle through the vector of data object types, and add on the size of each object data type

	std::vector<WONCommon::RawBuffer>::const_iterator data_object_types_iterator(m_data_object_types.begin());

	for (;
	     data_object_types_iterator != m_data_object_types.end();
		 data_object_types_iterator++)
		{
		// Add the size of the length of the data type

		result += sizeof(unsigned long);

		// Add the size of the data type itself

		result += data_object_types_iterator->size();
		}

	return result;
	}

void UserActivityDataTypes::pack_data(SmallMessage &reply_message) const
	{
	// Append the number of data object types

	reply_message.AppendShort(m_i_number_of_data_object_types);

	// Cycle through the vector of data object types, and append each data object type

	std::vector<WONCommon::RawBuffer>::const_iterator data_object_types_iterator(m_data_object_types.begin());

	for (;
	     data_object_types_iterator != m_data_object_types.end();
		 data_object_types_iterator++)
		{
		// Append the length of the data type

		reply_message.AppendLong(data_object_types_iterator->size());

		// Append the data type itself

		reply_message.AppendBytes(data_object_types_iterator->size(), 
								  data_object_types_iterator->data());
		}
	}

void UserActivityDataTypes::unpack_data(const SmallMessage &reply_message)
	{
	// Retrieve the number of data object types

	m_i_number_of_data_object_types = reply_message.ReadShort();

	// Resize the vector to store the correct number of data objects

	m_data_object_types.resize(m_i_number_of_data_object_types);

	// Cycle through the vector of data object types, and retrieve each data object type

	std::vector<WONCommon::RawBuffer>::iterator data_object_types_iterator(m_data_object_types.begin());

	for (;
	     data_object_types_iterator != m_data_object_types.end();
		 data_object_types_iterator++)
		{
		// Retrieve the length of the data type

		unsigned long data_size = reply_message.ReadLong();

		// Retrieve the data type itself

		data_object_types_iterator->assign(reinterpret_cast<const unsigned char*>(reply_message.ReadBytes(data_size)), 
									 	   data_size);
		}
	}
