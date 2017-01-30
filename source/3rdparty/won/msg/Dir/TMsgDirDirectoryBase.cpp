// TMsgDirDirectoryBase.h

// Base class for directory server sub directory messages.  This class is
// not a directory server message itself.  It is just further refinement
// of TMessage for use in directory messages.


#include "common/won.h"
#include "msg/TMessage.h"
#include "TMsgDirPeerDataBase.h"
#include "TMsgDirDirectoryBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirPeerDataBase;
	using WONMsg::TMsgDirDirectoryBase;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirDirectoryBase::TMsgDirDirectoryBase(void) :
	TMsgDirPeerDataBase(),
	mDirectoryPath()
{}


// TMessage ctor
TMsgDirDirectoryBase::TMsgDirDirectoryBase(const TMessage& theMsgR) :
	TMsgDirPeerDataBase(theMsgR),
	mDirectoryPath()
{}


// Copy ctor
TMsgDirDirectoryBase::TMsgDirDirectoryBase(const TMsgDirDirectoryBase& theMsgR) :
	TMsgDirPeerDataBase(theMsgR),
	mDirectoryPath(theMsgR.mDirectoryPath)
{}


// Destructor
TMsgDirDirectoryBase::~TMsgDirDirectoryBase(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirDirectoryBase&
TMsgDirDirectoryBase::operator=(const TMsgDirDirectoryBase& theMsgR)
{
	TMsgDirPeerDataBase::operator=(theMsgR);
	mDirectoryPath    = theMsgR.mDirectoryPath;
	return *this;
}


// TMsgDirDirectoryBase::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirDirectoryBase::Pack(void)
{
	WTRACE("TMsgDirDirectoryBase::Pack");
	TMsgDirPeerDataBase::Pack();

	WDBG_LL("TMsgDirDirectoryBase::Pack Appending message data");
	Append_PW_STRING(mDirectoryPath);

	return GetDataPtr();
}


// TMsgDirDirectoryBase::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirDirectoryBase::Unpack(void)
{
	WTRACE("TMsgDirDirectoryBase::Unpack");
	TMsgDirPeerDataBase::Unpack();

	WDBG_LL("TMsgDirDirectoryBase::Unpack Reading message data");
	ReadWString(mDirectoryPath);
}
