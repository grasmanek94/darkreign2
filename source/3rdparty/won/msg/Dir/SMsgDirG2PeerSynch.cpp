// SMsgDirG2PeerSynch.h

// Directory Peer Synchronization message class (generation 2).  Contains dir
// entities to be synchronized between DirServers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "DirEntity.h"
#include "SMsgDirG2EntityListBase.h"
#include "SMsgDirG2PeerSynch.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2EntityListBase;
	using WONMsg::SMsgDirG2PeerSynchTest;
	using WONMsg::SMsgDirG2PeerSynchPath;
	using WONMsg::SMsgDirG2PeerSynch;
};

// Class constants
const unsigned long SMsgDirG2PeerSynch::gGetFlags = (
	// Decomposition
	WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPSERVICES |

	// Common
	WONMsg::GF_ADDTYPE    | WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_ADDCREATED     |
	WONMsg::GF_ADDTOUCHED | WONMsg::GF_ADDLIFESPAN    | WONMsg::GF_ADDUIDS        |
	WONMsg::GF_ADDDOTYPE  | WONMsg::GF_ADDDODATA      | WONMsg::GF_ADDDATAOBJECTS |
	WONMsg::GF_ADDACLS    |

	// Directory
	WONMsg::GF_DIRADDPATH | WONMsg::GF_DIRADDNAME | WONMsg::GF_DIRADDVISIBLE |

	// Service
	WONMsg::GF_SERVADDPATH | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR
);


// ** SMsgDirG2PeerSynchTest **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2PeerSynchTest::SMsgDirG2PeerSynchTest(void) :
	SmallMessage(),
	mServId(),
	mDirPath(),
	mDirCRC(0),
	mServCt(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynchTest);
}


// SmallMessage ctor
SMsgDirG2PeerSynchTest::SMsgDirG2PeerSynchTest(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mServId(),
	mDirPath(),
	mDirCRC(0),
	mServCt(0)

{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerSynchTest::SMsgDirG2PeerSynchTest(const SMsgDirG2PeerSynchTest& theMsgR) :
	SmallMessage(theMsgR),
	mServId(theMsgR.mServId),
	mDirPath(theMsgR.mDirPath),
	mDirCRC(theMsgR.mDirCRC),
	mServCt(theMsgR.mServCt)

{}


// Destructor
SMsgDirG2PeerSynchTest::~SMsgDirG2PeerSynchTest(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerSynchTest&
SMsgDirG2PeerSynchTest::operator=(const SMsgDirG2PeerSynchTest& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mServId  = theMsgR.mServId;
	mDirPath = theMsgR.mDirPath;
	mDirCRC  = theMsgR.mDirCRC;
	mServCt  = theMsgR.mServCt;
	return *this;
}


// SMsgDirG2PeerSynchTest::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2PeerSynchTest::Pack(void)
{
	WTRACE("SMsgDirG2PeerSynchTest::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynchTest);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2PeerSynchTest::Pack Appending message data");
	Append_PA_STRING(mServId);
	Append_PW_STRING(mDirPath);
	AppendLong(mDirCRC);
	AppendShort(mServCt);

	return GetDataPtr();
}


// SMsgDirG2PeerSynchTest::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerSynchTest::Unpack(void)
{
	WTRACE("SMsgDirG2PeerSynchTest::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerSynchTest))
	{
		WDBG_AH("SMsgDirG2PeerSynchTest::Unpack Not a DirG2PeerSynchTest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2PeerSynchTest message.");
	}

	WDBG_LL("SMsgDirG2PeerSynchTest::Unpack Reading message data");
	ReadString(mServId);
	ReadWString(mDirPath);
	mDirCRC = ReadLong();
	mServCt = ReadShort();
}


// ** SMsgDirG2PeerSynchPath **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2PeerSynchPath::SMsgDirG2PeerSynchPath(void) :
	SmallMessage(),
	mServId(),
	mDirPath()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynchPath);
}


// SmallMessage ctor
SMsgDirG2PeerSynchPath::SMsgDirG2PeerSynchPath(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mServId(),
	mDirPath()
{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerSynchPath::SMsgDirG2PeerSynchPath(const SMsgDirG2PeerSynchPath& theMsgR) :
	SmallMessage(theMsgR),
	mServId(theMsgR.mServId),
	mDirPath(theMsgR.mDirPath)
{}


// Destructor
SMsgDirG2PeerSynchPath::~SMsgDirG2PeerSynchPath(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerSynchPath&
SMsgDirG2PeerSynchPath::operator=(const SMsgDirG2PeerSynchPath& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mServId  = theMsgR.mServId;
	mDirPath = theMsgR.mDirPath;
	return *this;
}


// SMsgDirG2PeerSynchPath::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2PeerSynchPath::Pack(void)
{
	WTRACE("SMsgDirG2PeerSynchPath::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynchPath);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2PeerSynchPath::Pack Appending message data");
	Append_PA_STRING(mServId);
	Append_PW_STRING(mDirPath);

	return GetDataPtr();
}


// SMsgDirG2PeerSynchPath::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerSynchPath::Unpack(void)
{
	WTRACE("SMsgDirG2PeerSynchPath::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerSynchPath))
	{
		WDBG_AH("SMsgDirG2PeerSynchPath::Unpack Not a DirG2PeerSynchPath message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2PeerSynchPath message.");
	}

	WDBG_LL("SMsgDirG2PeerSynchPath::Unpack Reading message data");
	ReadString(mServId);
	ReadWString(mDirPath);
}


// ** SMsgDirG2PeerSynch **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2PeerSynch::SMsgDirG2PeerSynch(void) :
	SMsgDirG2EntityListBase(),
	mServId()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynch);
}


// SmallMessage ctor
SMsgDirG2PeerSynch::SMsgDirG2PeerSynch(const SmallMessage& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mServId()
{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerSynch::SMsgDirG2PeerSynch(const SMsgDirG2PeerSynch& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mServId(theMsgR.mServId)
{}


// Destructor
SMsgDirG2PeerSynch::~SMsgDirG2PeerSynch(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerSynch&
SMsgDirG2PeerSynch::operator=(const SMsgDirG2PeerSynch& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SMsgDirG2EntityListBase::operator=(theMsgR);
		mServId = theMsgR.mServId;
	}
	return *this;
}


// SMsgDirG2PeerSynch::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2PeerSynch::Pack(void)
{
	WTRACE("SMsgDirG2PeerSynch::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerSynch);
	SMsgDirG2EntityListBase::Pack();

	WDBG_LL("SMsgDirG2PeerSynch::Pack Appending message data");
	Append_PA_STRING(mServId);

	PackEntities();

	return GetDataPtr();
}


// SMsgDirG2PeerSynch::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerSynch::Unpack(void)
{
	WTRACE("SMsgDirG2PeerSynch::Unpack");
	SMsgDirG2EntityListBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerSynch))
	{
		WDBG_AH("SMsgDirG2PeerSynch::Unpack Not a DirG2PeerSynch message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2PeerSynch message.");
	}

	WDBG_LL("SMsgDirG2PeerSynch::Unpack Reading message data");
	ReadString(mServId);

	UnpackEntities();
}


unsigned long
SMsgDirG2PeerSynch::ComputePackSize() const
{
	return (SMsgDirG2EntityListBase::ComputePackSize() + ComputeStringPackSize(mServId));
}
