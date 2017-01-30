#ifndef __SMsgUsrSubscribeToUserList_H__
#define __SMsgUsrSubscribeToUserList_H__

// SMsgUsrSubscribeToUserList.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

#define SMsgUsrSubscribeToUserListTYPE WONMsg::EUserSearchSubscribeToPushedUserList

namespace WONMsg 
	{
	class SMsgUsrSubscribeToUserList : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrSubscribeToUserList(const unsigned long &i_data_source_id = 0);

			// SmallMessage ctor

			explicit SMsgUsrSubscribeToUserList(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrSubscribeToUserList(const SMsgUsrSubscribeToUserList& source_message);

			// Destructor

			~SMsgUsrSubscribeToUserList(void);

			// Assignment

			SMsgUsrSubscribeToUserList& operator=(const SMsgUsrSubscribeToUserList& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

			// Accessors for the private data

			const unsigned long &get_data_source_id(void) const;

			unsigned long &get_data_source_id(void);

		private:

			unsigned long m_i_data_source_id;  // List of directory paths/modes
		};

	// Inlines

	inline TRawMsg* SMsgUsrSubscribeToUserList::Duplicate(void) const
		{ 
		return new SMsgUsrSubscribeToUserList(*this); 
		}

	inline const unsigned long &SMsgUsrSubscribeToUserList::get_data_source_id(void) const
		{ 
		return m_i_data_source_id; 
		}

	inline unsigned long &SMsgUsrSubscribeToUserList::get_data_source_id(void)
		{ 
		return m_i_data_source_id; 
		}

	};  // Namespace WONMsg

#endif