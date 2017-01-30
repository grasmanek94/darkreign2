#ifndef __SMsgUsrGetNumUsers_H__
#define __SMsgUsrGetNumUsers_H__

// SMsgUsrGetNumUsers.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg 
	{
	class SMsgUsrGetNumUsers : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrGetNumUsers(const wstring &category_to_find_users_from = L"");

			// SmallMessage ctor

			explicit SMsgUsrGetNumUsers(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrGetNumUsers(const SMsgUsrGetNumUsers& source_message);

			// Destructor

			~SMsgUsrGetNumUsers(void);

			// Assignment

			SMsgUsrGetNumUsers& operator=(const SMsgUsrGetNumUsers& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

			// Accessors for the private data

			const wstring &get_category_name(void) const;

			wstring &get_category_name(void);

		private:

			wstring m_s_category_name;  // List of directory paths/modes
		};

	// Inlines

	inline TRawMsg* SMsgUsrGetNumUsers::Duplicate(void) const
		{ 
		return new SMsgUsrGetNumUsers(*this); 
		}

	inline const wstring &SMsgUsrGetNumUsers::get_category_name(void) const
		{ 
		return m_s_category_name; 
		}

	inline wstring &SMsgUsrGetNumUsers::get_category_name(void)
		{ 
		return m_s_category_name; 
		}

	};  // Namespace WONMsg

#endif