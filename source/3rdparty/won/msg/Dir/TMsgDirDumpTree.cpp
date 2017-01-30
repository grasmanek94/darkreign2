// TMsgDirDumpTree.h

// Directory Dump Tree Message class.  Outputs the Directory Server internal
// directory/service tree to a specified file (path).  Message specifies the
// file name to output the tree to and an option path to a directory to write
// the output file in.  If the directory is NULL, file will be output in the
// "C:\temp" directory.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirDumpTree.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirDumpTree;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirDumpTree::TMsgDirDumpTree(void) :
	TMessage(),
	mFilePath(),
	mFileName()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirDumpTree);
}


// TMessage ctor
TMsgDirDumpTree::TMsgDirDumpTree(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mFilePath(),
	mFileName()
{
	Unpack();
}


// Copy ctor
TMsgDirDumpTree::TMsgDirDumpTree(const TMsgDirDumpTree& theMsgR) :
	TMessage(theMsgR),
	mFilePath(theMsgR.mFilePath),
	mFileName(theMsgR.mFileName)
{}


// Destructor
TMsgDirDumpTree::~TMsgDirDumpTree(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirDumpTree&
TMsgDirDumpTree::operator=(const TMsgDirDumpTree& theMsgR)
{
	TMessage::operator=(theMsgR);
	mFilePath = theMsgR.mFilePath;
	mFileName = theMsgR.mFileName;
	return *this;
}


// TMsgDirDumpTree::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirDumpTree::Pack(void)
{
	WTRACE("TMsgDirGetDirContents::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirDumpTree);
	TMessage::Pack();

	WDBG_LL("TMsgDirDumpTree::Pack Appending message data");
	Append_PA_STRING(mFilePath);
	Append_PA_STRING(mFileName);

	return GetDataPtr();
}


// TMsgDirDumpTree::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirDumpTree::Unpack(void)
{
	WTRACE("TMsgDirDumpTree::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirDumpTree))
	{
		WDBG_AH("TMsgDirDumpTree::Unpack Not a DirDumpTree message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirDumpTree message.");
	}

	WDBG_LL("TMsgDirDumpTree::Unpack Reading message data");
	ReadString(mFilePath);
	ReadString(mFileName);
}
