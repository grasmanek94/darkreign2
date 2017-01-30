#ifndef __SMsgUsrRemoveDataSource_H__
#define __SMsgUsrRemoveDataSource_H__

// SMsgUsrRemoveDataSource.h

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg 
	{
	class SMsgUsrRemoveDataSource : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrRemoveDataSource(const unsigned long &i_data_source_id = 0);

			// SmallMessage ctor

			explicit SMsgUsrRemoveDataSource(const SmallMessage& theMsgR);

			// Copy ctor

			SMsgUsrRemoveDataSource(const SMsgUsrRemoveDataSource& theMsgR);

			// Destructor

			~SMsgUsrRemoveDataSource(void);

			// Assignment

			SMsgUsrRemoveDataSource& operator=(const SMsgUsrRemoveDataSource& theMsgR);

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

	inline const unsigned long &SMsgUsrRemoveDataSource::get_data_source_id() const
		{ 
		return m_i_data_source_id; 
		}

	inline TRawMsg*	SMsgUsrRemoveDataSource::Duplicate() const
		{ 
		return new SMsgUsrRemoveDataSource(*this); 
		}

	};  // Namespace WONMsg

#endif