#ifndef _TMsgDirLoadFromSource_H
#define _TMsgDirLoadFromSource_H

// TMsgDirLoadFromSource.h

// DirectoryServer Load From Source message.  Requests directory tree of a source
// Directory Server.

#include "msg/TMessage.h"

namespace WONMsg {

class TMsgDirLoadFromSource : public TMessage
{
public:
	// Default ctor
	TMsgDirLoadFromSource(void);

	// TMessage ctor
	explicit TMsgDirLoadFromSource(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirLoadFromSource(const TMsgDirLoadFromSource& theMsgR);

	// Destructor
	~TMsgDirLoadFromSource(void);

	// Assignment
	TMsgDirLoadFromSource& operator=(const TMsgDirLoadFromSource& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

private:
};


// Inlines
inline TRawMsg*
TMsgDirLoadFromSource::Duplicate(void) const
{ return new TMsgDirLoadFromSource(*this); }

};  // Namespace WONMsg

#endif