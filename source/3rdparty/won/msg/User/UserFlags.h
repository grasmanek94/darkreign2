#ifndef __USERFLAGS_H__
#define __USERFLAGS_H__

// UserFlags

// Defines various enums for the flags used in User Search messages.

// In the WONMsg namespace

namespace WONMsg 
	{
	// FindFlags (byte) - Control flags for find queries

	enum UserFindFlags
		{
		USFF_NOFLAGS   = 0,

		USFF_SEND_ALL_DATA_OBJECTS  = 0x01,  // Return all data objects,
										   // ignoring the activity data
										   // type data in the query

		USFF_SEND_WON_USERS_ONLY    = 0x02,  // Send only WON users

		USFF_SEND_FIRST_USER_ONLY   = 0x04,  // Send the first found user
										   // only

		USFF_ALLFLAGS  = 0xff
		};

	// The different user status'

	typedef enum {E_Status_Not_Available		= 0,
				  E_Status_Playing				= 1,
				  E_Status_Chatting				= 2,
				  E_Status_Waiting_To_Play		= 3}
			EStatusType;

	// FindMatchMode (byte) - Mode for find queries

	enum UserFindMatchMode
		{
		USFMM_EXACT				= 0,  // Compared value must equal search value. Applies
									  // to all search keys

		USFMM_BEGIN				= 1,  // Compared value must begin with search value. Applies
									  // to name and binary search keys

		USFMM_CONTAIN				= 2,  // Compared value must contain search value. Applies
									  // to name and binary search keys

		USFMM_LESS_THAN			= 3,  // Search key must be less than compared value. Applies
									  // to binary search keys only.

		USFMM_LESS_THAN_EQUAL		= 4,  // Search key must be less than or equal to compared value. 
									  // Applies to binary search keys only.

		USFMM_GREATER_THAN		= 5,  // Search key must be greater than compared value. Applies
									  // to binary search keys only.

		USFMM_GREATER_THAN_EQUAL	= 6,  // Search key must be greater than or equal to compared value. 
									  // Applies to binary search keys only.

		USFMM_EQUAL_WILDCARD		= 7,  // Compared value must equal search value (with wildcard support).
									  // Applies to name search keys only.

		USFMM_MAX
		};

	// The different key types

	typedef enum {E_Key_Type_User_Name,
				  E_Key_Type_Category_Name,
				  E_Key_Type_WON_User_Id,
				  E_Key_Type_Community_Id,
				  E_Key_Type_Community_Name,
				  E_Key_Type_User_Status,
				  E_Key_Type_Location,
				  E_Key_Type_Binary_Key}
			EKeyType;
				  
	};  // Namespace WONMsg

#endif