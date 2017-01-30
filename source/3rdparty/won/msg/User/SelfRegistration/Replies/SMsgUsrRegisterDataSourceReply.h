#ifndef __SMsgUsrRegisterDataSourceReply_H__
#define __SMsgUsrRegisterDataSourceReply_H__

// SMsgUsrRegisterDataSourceReply.h

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg 
	{
	class SMsgUsrRegisterDataSourceReply : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrRegisterDataSourceReply(const unsigned long &i_data_source_id = 0);

			// SmallMessage ctor

			explicit SMsgUsrRegisterDataSourceReply(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrRegisterDataSourceReply(const SMsgUsrRegisterDataSourceReply& theMsgR);

			// Destructor

			~SMsgUsrRegisterDataSourceReply(void);

			// Assignment

			SMsgUsrRegisterDataSourceReply& operator=(const SMsgUsrRegisterDataSourceReply& theMsgR);

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

			// Accessors for the private data

			unsigned long &get_data_source_id(void)
				{ 
				return m_i_data_source_id; 
				}

		protected:

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate() const;

		private:

			unsigned long m_i_data_source_id;  // The data source ID for the
											   // self-registered data source
		};

	// Inlines

	inline TRawMsg*	SMsgUsrRegisterDataSourceReply::Duplicate() const
		{ 
		return new SMsgUsrRegisterDataSourceReply(*this); 
		}

	};  // Namespace WONMsg

#endif