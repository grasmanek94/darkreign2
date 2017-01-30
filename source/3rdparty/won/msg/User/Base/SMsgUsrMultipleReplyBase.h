#ifndef __SMsgUsrMultipleReplyBase_H__
#define __SMsgUsrMultipleReplyBase_H__

// SMsgUsrMultipleReplyBase.h

// User Status Reply class.  Returns the status of a
// request made to the User Server.

#include "SMsgUsrReplyBase.h"

// Forwards from WONSocket

namespace WONMsg 
	{
	class SMsgUsrMultipleReplyBase : public SMsgUsrReplyBase
		{
		public:

			// Default ctor

			SMsgUsrMultipleReplyBase(const unsigned long &service_type,
									 const unsigned long &message_type,
									 const unsigned char &i_reply_sequence_number,
									 const ServerStatus &server_status = WONMsg::StatusCommon_Success);

			// TMessage ctor - will throw if SmallMessage type is not of this type

			explicit SMsgUsrMultipleReplyBase(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrMultipleReplyBase(const SMsgUsrMultipleReplyBase& source_message);

			// Destructor

			~SMsgUsrMultipleReplyBase(void);

			// Assignment

			SMsgUsrMultipleReplyBase& operator=(const SMsgUsrMultipleReplyBase& source_message);

			// Reply message specific data

			const unsigned char &get_reply_sequence_number() const;

			unsigned char &get_reply_sequence_number();

			virtual unsigned long get_packed_size_in_bytes() const
				{
				return (SMsgUsrReplyBase::get_packed_size_in_bytes() + sizeof(unsigned char));
				}

		protected:

			virtual void pack_multiple_message_specific_data() = 0;
			
			virtual void unpack_multiple_message_specific_data() = 0;

		private:

			virtual void pack_message_specific_data();
			
			virtual void unpack_message_specific_data();

			unsigned char m_i_reply_sequence;  // Reply sequence. Bits 0-6 are the 
											   // sequence number. Bit 7 is the
											   // last reply flag.
		};

		// Inlines

		inline const unsigned char &SMsgUsrMultipleReplyBase::get_reply_sequence_number() const
			{ 
			return m_i_reply_sequence; 
			}

		inline unsigned char &SMsgUsrMultipleReplyBase::get_reply_sequence_number()
			{ 
			return m_i_reply_sequence; 
			}

	};  // Namespace WONMsg

#endif