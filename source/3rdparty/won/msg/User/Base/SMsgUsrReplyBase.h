#ifndef __SMsgUsrReplyBase_H__
#define __SMsgUsrReplyBase_H__

// SMsgUsrReplyBase.h

// User Status Reply class.  Returns the status of a
// request made to the User Server.

#include <string>
#include "msg/ServerStatus.h"
#include "msg/SServiceTypes.h"
#include "msg/TMessage.h"
#include "msg/User/SmallMessageTypesUserSearch.h"

// Forwards from WONSocket

namespace WONMsg 
	{
	class SMsgUsrReplyBase : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrReplyBase(const unsigned long &service_type,
											const unsigned long &message_type,
											const ServerStatus &server_status = WONMsg::StatusCommon_Success);

			// TMessage ctor - will throw if SmallMessage type is not of this type

			explicit SMsgUsrReplyBase(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrReplyBase(const SMsgUsrReplyBase& source_message);

			// Destructor

			~SMsgUsrReplyBase(void);

			// Assignment

			SMsgUsrReplyBase& operator=(const SMsgUsrReplyBase& source_message);

			// Status access

			ServerStatus GetStatus(void) const;

			void         SetStatus(ServerStatus server_status);

			virtual unsigned long get_packed_size_in_bytes() const
				{
				return (SmallMessage::GetHeaderLength() + sizeof(ServerStatus));
				}

			// Pack and Unpack the message
			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(); 

			void  Unpack();

		protected:

			virtual void pack_message_specific_data() = 0;
			
			virtual void unpack_message_specific_data() = 0;

			virtual unsigned long get_service_type() const = 0;

			virtual unsigned long get_message_type() const = 0;

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate(void) const = 0;

		private:

			ServerStatus         m_server_status;   // Request status
		};

	inline ServerStatus SMsgUsrReplyBase::GetStatus(void) const
		{ 
		return m_server_status; 
		}

	inline void	SMsgUsrReplyBase::SetStatus(ServerStatus server_status)
		{ 
		m_server_status = server_status; 
		}
	};  // Namespace WONMsg

#endif