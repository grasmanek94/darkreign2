// SMsgDirG2DumpTree.h

// Directory Dump Tree Message class.  Outputs the Directory Server internal
// directory/service tree to a specified file (path).  Message specifies the
// file name to output the tree to and an option path to a directory to write
// the output file in.  If the directory is NULL, file will be output in the
// "C:\temp" directory.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2DumpTree.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2DumpTree;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2DumpTree::SMsgDirG2DumpTree(void) :
	SmallMessage(),
	mFilePath(),
	mFileName()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2DumpTree);
}


// SmallMessage ctor
SMsgDirG2DumpTree::SMsgDirG2DumpTree(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mFilePath(),
	mFileName()
{
	Unpack();
}


// Copy ctor
SMsgDirG2DumpTree::SMsgDirG2DumpTree(const SMsgDirG2DumpTree& theMsgR) :
	SmallMessage(theMsgR),
	mFilePath(theMsgR.mFilePath),
	mFileName(theMsgR.mFileName)
{}


// Destructor
SMsgDirG2DumpTree::~SMsgDirG2DumpTree(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2DumpTree&
SMsgDirG2DumpTree::operator=(const SMsgDirG2DumpTree& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mFilePath = theMsgR.mFilePath;
	mFileName = theMsgR.mFileName;
	return *this;
}


// SMsgDirG2DumpTree::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2DumpTree::Pack(void)
{
	WTRACE("SMsgDirG2DumpTree::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2DumpTree);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2DumpTree::Pack Appending message data");
	Append_PA_STRING(mFilePath);
	Append_PA_STRING(mFileName);

	return GetDataPtr();
}


// SMsgDirG2DumpTree::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2DumpTree::Unpack(void)
{
	WTRACE("SMsgDirG2DumpTree::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2DumpTree))
	{
		WDBG_AH("SMsgDirG2DumpTree::Unpack Not a DirG2DumpTree message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2DumpTree message.");
	}

	WDBG_LL("SMsgDirG2DumpTree::Unpack Reading message data");
	ReadString(mFilePath);
	ReadString(mFileName);
}
