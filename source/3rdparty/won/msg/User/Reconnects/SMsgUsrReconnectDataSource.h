#ifndef __SMsgUsrReconnectDataSource_H__
#define __SMsgUsrReconnectDataSource_H__

// SMsgUsrReconnectDataSource.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

#define SMsgUsrReconnectDataSourceTYPE WONMsg::EUserSearchReconnectDataSource

namespace WONMsg 
	{
	class SMsgUsrReconnectDataSource : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrReconnectDataSource(const unsigned long &i_data_source_id = 0);

			// SmallMessage ctor

			explicit SMsgUsrReconnectDataSource(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrReconnectDataSource(const SMsgUsrReconnectDataSource& source_message);

			// Destructor

			~SMsgUsrReconnectDataSource(void);

			// Assignment

			SMsgUsrReconnectDataSource& operator=(const SMsgUsrReconnectDataSource& source_message);

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

	inline TRawMsg* SMsgUsrReconnectDataSource::Duplicate(void) const
		{ 
		return new SMsgUsrReconnectDataSource(*this); 
		}

	inline const unsigned long &SMsgUsrReconnectDataSource::get_data_source_id(void) const
		{ 
		return m_i_data_source_id; 
		}

	inline unsigned long &SMsgUsrReconnectDataSource::get_data_source_id(void)
		{ 
		return m_i_data_source_id; 
		}

	};  // Namespace WONMsg

#endif