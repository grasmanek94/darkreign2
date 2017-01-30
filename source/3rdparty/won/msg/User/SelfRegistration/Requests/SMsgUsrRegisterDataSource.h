#ifndef __SMsgUsrRegisterDataSource_H__
#define __SMsgUsrRegisterDataSource_H__

// SMsgUsrRegisterDataSource.h

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.

#include <string>
#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg 
	{
	class SMsgUsrRegisterDataSource : public SmallMessage
		{
		public:

			// Default ctor

			SMsgUsrRegisterDataSource(const wstring &s_category_to_join = L"",
									  const wstring &s_type_name = L"");

			// SmallMessage ctor

			explicit SMsgUsrRegisterDataSource(const SmallMessage& source_message);

			// Copy ctor

			SMsgUsrRegisterDataSource(const SMsgUsrRegisterDataSource& source_message);

			// Destructor

			~SMsgUsrRegisterDataSource(void);

			// Assignment

			SMsgUsrRegisterDataSource& operator=(const SMsgUsrRegisterDataSource& source_message);

			// Virtual Duplicate from TMessage

			TRawMsg* Duplicate(void) const;

			// Pack and Unpack the message

			// Unpack will throw a BadMsgException is message is not of this type

			void* Pack(void); 

			void  Unpack(void);

			// Accessors for the private data

			const wstring &get_category_name(void) const
				{ 
				return m_s_category_name; 
				}

			const wstring &get_type_name(void) const
				{ 
				return m_s_type_name; 
				}

		private:

			wstring m_s_category_name;  // Category to place data source into
			wstring m_s_type_name;		// The typename for this data source
		};

	// Inlines

	inline TRawMsg* SMsgUsrRegisterDataSource::Duplicate(void) const
		{ 
		return new SMsgUsrRegisterDataSource(*this); 
		}

	};  // Namespace WONMsg

#endif