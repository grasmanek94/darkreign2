#ifndef __SMsgUsrReconnectSelfDataSource_H__
#define __SMsgUsrReconnectSelfDataSource_H__

// SMsgUsrReconnectSelfDataSource.h

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg 
	{
	class SMsgUsrReconnectSelfDataSource : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrReconnectSelfDataSource(const unsigned long &i_data_source_id = 0);

			// SmallMessage ctor

			explicit SMsgUsrReconnectSelfDataSource(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrReconnectSelfDataSource(const SMsgUsrReconnectSelfDataSource& theMsgR);

			// Destructor

			~SMsgUsrReconnectSelfDataSource(void);

			// Assignment

			SMsgUsrReconnectSelfDataSource& operator=(const SMsgUsrReconnectSelfDataSource& theMsgR);

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

			// Accessors for the private data

			const unsigned long &get_data_source_id(void) const;

		protected:

			// Virtual Duplicate from TMessage

			virtual TRawMsg* Duplicate() const;

		private:

			unsigned long m_i_data_source_id;  // The data source ID for the
											   // self-registered data source
											   // to be removed
		};

	// Inlines

	inline const unsigned long &SMsgUsrReconnectSelfDataSource::get_data_source_id() const
		{ 
		return m_i_data_source_id; 
		}

	inline TRawMsg*	SMsgUsrReconnectSelfDataSource::Duplicate() const
		{ 
		return new SMsgUsrReconnectSelfDataSource(*this); 
		}

	};  // Namespace WONMsg

#endif