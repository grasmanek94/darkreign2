
#include "common/won.h"
#include "BadMsgException.h"
#include "TMessage.h"

// Titan messages base classes w/ reference counting

namespace {
	using WONMsg::BadMsgException;
	using WONMsg::TRawData;
	using WONMsg::TRawMsg;
	using WONMsg::BaseMessage;
	using WONMsg::TMessage;
	using WONMsg::MiniMessage;
	using WONMsg::SmallMessage;
	using WONMsg::LargeMessage;
	using WONMsg::HeaderMessage;

	const unsigned long CHUNKSIZE = 512;
};

// TRawData, the actual object that is reference counted against
////////////////////////////////////////////////////////////////

// Default Constructor
TRawData::TRawData() :
	mRefCount(1),
	mChunkMult(1),
	mDataP(NULL),
	mDataLen(0),
	mReadPtrP(NULL),
	mWritePtrP(NULL),
	mStrBuf(),
	mWStrBuf()
{}

// Data Constructor
TRawData::TRawData(unsigned long theDataLen, const void *theData) :
	mRefCount(1),
	mChunkMult(1),
	mDataP(NULL),
	mDataLen(0),
	mReadPtrP(NULL),
	mWritePtrP(NULL),
	mStrBuf(),
	mWStrBuf()
{
	if (theDataLen > 0)
	{
		AllocateNewBuffer(theDataLen);
		if (theData) memcpy(mDataP, theData, theDataLen);
	}
}

// Copy Constructor
TRawData::TRawData(const TRawData& theDataR) :
	mRefCount(1),
	mChunkMult(1),
	mDataP(NULL),
	mDataLen(0),
	mReadPtrP(NULL),
	mWritePtrP(NULL),
	mStrBuf(),
	mWStrBuf()
{
	if (theDataR.mDataLen > 0)
	{
		AllocateNewBuffer(theDataR.mDataLen);
		memcpy(mDataP, theDataR.mDataP, theDataR.mDataLen);

		mWritePtrP = mDataP + (theDataR.mWritePtrP - theDataR.mDataP);
		mReadPtrP = mDataP + (theDataR.mReadPtrP - theDataR.mDataP);
	}
}

// Destructor
TRawData::~TRawData()
{
	delete [] mDataP;
}

void * TRawData::AllocateNewBuffer(unsigned long theSize)
{
	if (theSize != mDataLen)
	{
		delete [] mDataP;
		mDataLen = theSize;
		mDataP = new unsigned char[theSize];
	}

	// Reset read and write pointers
	mReadPtrP  = mWritePtrP = mDataP;
	mChunkMult = 1;
	return mDataP;
}

// for now, append operations have no optimizations
// we implement a full re-allocation and copy
// nNewBytes = number of new bytes to add
void * TRawData::AddAllocate(unsigned long nNewBytes, bool chunkAlloc, bool doDumbCheck)
{
	unsigned long aNewLen = 0;

	// No buffer, alloc a new one of the requested size
	if (! mDataP)
		AllocateNewBuffer(nNewBytes);

	// Grow buffer if no writes have happened or requested size would be off
	// end of existing buffer.
	else if ((doDumbCheck && mWritePtrP == mDataP) ||
	         ((mWritePtrP + nNewBytes) > (mDataP + mDataLen)))
	{
		aNewLen = mDataLen + nNewBytes;
		if (chunkAlloc)
		{
			aNewLen += CHUNKSIZE * mChunkMult;
			mChunkMult *= 2;  // Double for next time
		}
	}

	// If buffer must be grown, do so.
	if (aNewLen > 0)
	{
		unsigned char* aTmpP = new unsigned char[aNewLen]; 
		memcpy(aTmpP, mDataP, mDataLen);

		mReadPtrP  = aTmpP + (mReadPtrP - mDataP);
		mWritePtrP = aTmpP + (mWritePtrP - mDataP);

		delete [] mDataP;
		mDataP   = aTmpP;
		mDataLen = aNewLen;
	}

	// Return the write position
	return mWritePtrP;
}

void TRawData::ResetBuffer(void)
{ 
	delete [] mDataP;
	mReadPtrP = mWritePtrP = mDataP = NULL;
	mDataLen = 0;
	mChunkMult = 1;
}

unsigned long TRawData::GetDataLen() const
{
	return (mWritePtrP > mDataP ? (mWritePtrP - mDataP) : mDataLen);
}

void TRawData::AppendLongLong(__int64 theLongLong)
{
	AddAllocate(sizeof(__int64), true);
	*(reinterpret_cast<__int64*>(mWritePtrP)) = WONCommon::htotll(theLongLong);
	mWritePtrP += sizeof(__int64);
}

void TRawData::AppendLong(unsigned long theLong)
{
	AddAllocate(sizeof(unsigned long), true);
	*(reinterpret_cast<unsigned long*>(mWritePtrP)) = WONCommon::htotl(theLong);
	mWritePtrP += sizeof(unsigned long);
}

void TRawData::AppendShort(unsigned short theShort)
{
	AddAllocate(sizeof(unsigned short), true);
	*(reinterpret_cast<unsigned short*>(mWritePtrP)) = WONCommon::htots(theShort);
	mWritePtrP += sizeof(unsigned short);
}

void TRawData::AppendByte(unsigned char theByte)
{
	AddAllocate(sizeof(unsigned char), true);
	*mWritePtrP = theByte;
	mWritePtrP += sizeof(unsigned char);
}

void TRawData::AppendBytes(long theNumBytes, const void* theBytesP, bool chunkAlloc, bool doDumbCheck)
{
	AddAllocate(theNumBytes, chunkAlloc, doDumbCheck);
	memcpy(mWritePtrP, theBytesP, theNumBytes);
	mWritePtrP += theNumBytes;
}

void TRawData::SkipWritePtrAhead(unsigned long theNumBytes)
{
	AddAllocate(theNumBytes, false, false);
	mWritePtrP+=theNumBytes;
}


//UNICODE
void TRawData::Append_PW_STRING(const wstring& theString)
{
	unsigned short aNumChar = theString.size();
	AppendShort(aNumChar);

	if (aNumChar > 0)
	{
		unsigned long aByteCt = aNumChar * sizeof(wchar);
		AddAllocate(aByteCt, true);

		WONCommon::htotUnicodeString(theString.data(), (wchar*)mWritePtrP, aNumChar);
		mWritePtrP += aByteCt;
	}
}

void TRawData::Append_PA_STRING(const string& theString)
{
	unsigned short aNumChar = theString.size();
	AppendShort(aNumChar);

	if (aNumChar > 0)
	{
		AddAllocate(aNumChar, true);
		memcpy(mWritePtrP, theString.c_str(), aNumChar);
		mWritePtrP += aNumChar;
	}
}

void TRawData::ReadWString(wstring& theBufR) const
{
	theBufR.erase();
	unsigned short aNumChar = ReadShort(); //may throw!

	if (aNumChar > 0)
	{
		WONCommon::WONString aBuf((wchar*)mReadPtrP, aNumChar);
		theBufR = aBuf.GetUnicodeString();
		unsigned long theLen = static_cast<unsigned long>(aNumChar) * sizeof(wchar);
		CheckLength(theLen);  //may throw!
		mReadPtrP += theLen;
	}
}

const wstring& TRawData::Read_PW_STRING() const
{
	ReadWString(mWStrBuf);
	return mWStrBuf;
}

void TRawData::ReadString(string& theBufR) const
{
	theBufR.erase();

	unsigned short aNumChar = ReadShort(); //may throw!

	if (aNumChar > 0)
	{
		CheckLength(aNumChar);  //may throw!
		theBufR.append((char *) mReadPtrP, aNumChar);
		mReadPtrP += aNumChar;
	}
}

const string& TRawData::Read_PA_STRING() const
{
	ReadString(mStrBuf);
	return mStrBuf;
}

__int64 TRawData::ReadLongLong() const
{
	CheckLength(sizeof(__int64));  //may throw!
	__int64 theLongLong = *(reinterpret_cast<__int64*>(mReadPtrP));
	mReadPtrP += sizeof(__int64); //increment
	return WONCommon::ttohll(theLongLong); 
}

unsigned long TRawData::ReadLong() const
{
	CheckLength(sizeof(unsigned long));  //may throw!
	unsigned long theLong = *(reinterpret_cast<unsigned long*>(mReadPtrP));
	mReadPtrP += sizeof(unsigned long); //increment
	return WONCommon::ttohl(theLong); 
}

unsigned short TRawData::ReadShort() const
{ 
	CheckLength(sizeof(unsigned short));  //may throw!
	unsigned short theShort = *(reinterpret_cast<unsigned short*>(mReadPtrP));
	mReadPtrP += sizeof(unsigned short); //increment
	return WONCommon::ttohs(theShort); 
}

unsigned char TRawData::ReadByte() const
{ 
	CheckLength(sizeof(unsigned char));  //may throw!
	unsigned char theByte = *mReadPtrP;
	mReadPtrP += sizeof(unsigned char); //increment
	return theByte; 
}

const void * TRawData::ReadBytes(long theNumBytes) const
{ 
	CheckLength(theNumBytes);  //may throw!
	void * theBytes = mReadPtrP;
	mReadPtrP += theNumBytes; //increment
	return theBytes; 
}

// Don't advance read pointer on the peek..
const void * TRawData::PeekBytes(long theNumBytes) const
{ 
	CheckLength(theNumBytes);  //may throw!
	void * theBytes = mReadPtrP;
	return theBytes; 
}

unsigned long TRawData::BytesLeftToRead() const
{
	unsigned long aBytesTotal = GetDataLen();
	unsigned long aBytesRead  = mReadPtrP - mDataP;
	return (aBytesRead < aBytesTotal ? aBytesTotal - aBytesRead : 0);
}

void TRawData::CheckLength(unsigned long theLen) const
{
	const unsigned char* anEndP    = mDataP + GetDataLen();
	const unsigned char* aReadEndP = mReadPtrP + theLen;

	if (aReadEndP > anEndP)
		throw BadMsgException(WONCommon::ExBadTitanMessage, __LINE__, __FILE__,
		                      "Attempt to read past end of message!");
}

// TRawMsg implementation /////////////////////////////////////////////

TRawMsg::TRawMsg() :
	rep(new TRawData())
{}

TRawMsg::TRawMsg(unsigned long theDataLen, const void *theData) :
	rep(new TRawData(theDataLen, theData))
{}

TRawMsg::TRawMsg(const TRawMsg& Rmsg) :
	rep(Rmsg.rep)
{
#ifdef WIN32
	InterlockedIncrement(&(rep->mRefCount));
#else
	rep->mRefCountCrit.Enter();
	rep->mRefCount++;
	rep->mRefCountCrit.Leave();
#endif
}

TRawMsg::~TRawMsg()
{
	long result;
#ifdef WIN32
	result = InterlockedDecrement(&(rep->mRefCount));
#else
	rep->mRefCountCrit.Enter();
	result = --(rep->mRefCount);
	rep->mRefCountCrit.Leave();
#endif
	if (result <= 0)
		delete rep;
}

TRawMsg& TRawMsg::operator=(const TRawMsg& theRawMsgR) //shallow copy
{
#ifdef WIN32
	InterlockedIncrement(&(theRawMsgR.rep->mRefCount));	// protect against A = A
#else
	theRawMsgR.rep->mRefCountCrit.Enter();
	theRawMsgR.rep->mRefCount++;	// protect against A = A
	theRawMsgR.rep->mRefCountCrit.Leave();
#endif

	long result;
#ifdef WIN32
	result = InterlockedDecrement(&(rep->mRefCount));
#else
	rep->mRefCountCrit.Enter();
	result = --(rep->mRefCount);
	rep->mRefCountCrit.Leave();
#endif
	if (result <= 0)
		delete rep;
	
	rep = theRawMsgR.rep;	//share representation
	return *this;
}

TRawMsg* TRawMsg::Duplicate() const
{
	return new TRawMsg(*this);
}

void TRawMsg::Unpack()
{}

void * TRawMsg::Pack() 
{ 
	return rep->mDataP;
}

const void * TRawMsg::GetDataPtr() const
{
	return rep->mDataP;
}

void * TRawMsg::GetDataPtr()
{
	return rep->mDataP;
}

void TRawMsg::CopyOnWrite()
{
	if (rep->mRefCount > 1)
	{
		TRawData* aTmpP = rep;
		rep = new TRawData(*aTmpP);
#ifdef WIN32
		InterlockedDecrement(&(aTmpP->mRefCount));
#else
		aTmpP->mRefCountCrit.Enter();
		aTmpP->mRefCount--;
		aTmpP->mRefCountCrit.Leave();
#endif
	}
}

// BaseMessage implementation /////////////////////////////////////
BaseMessage::BaseMessage() 
{}

BaseMessage::BaseMessage(const BaseMessage& Tmsg) :
	TRawMsg(Tmsg),
	mMessageClass(Tmsg.mMessageClass)
{}

BaseMessage::BaseMessage(unsigned long theDataLen, const void *theData) :
	TRawMsg( theDataLen, theData )
{}

BaseMessage::~BaseMessage()
{}

BaseMessage& BaseMessage::operator= (const BaseMessage& theMsgR)
{
	TRawMsg::operator=(theMsgR);
	
	mMessageClass = theMsgR.mMessageClass;

	return *this;
}

void* BaseMessage::AllocateBodyBuffer(unsigned long size)
{
	if (size < GetHeaderLength() || size > MAXMSG_SIZE) //brain damaged header
	{
		WDBG_LH("BaseMessage::AllocateBodyBuffer bad message length, len=" << size);
		BadMsgException anEx(WONCommon::ExBadTitanMessage, __LINE__, __FILE__);
		anEx.GetStream() << "MsgLen=" << size;
		throw anEx;
	}

	return AddAllocate(size - GetHeaderLength());
}

void BaseMessage::Unpack()
{ 
	UnpackHeader();
}

void BaseMessage::UnpackHeader() const
{
	ResetReadPointer();
	ReadBytes(GetHeaderLength());
}

void* BaseMessage::Pack() 
{
	ResetWritePointer();
	SkipWritePtrAhead(GetHeaderLength());

	return GetDataPtr();
}

BaseMessage* BaseMessage::GetMsgOfType(unsigned char theHeaderType) 
{
	HeaderType aHeaderType = (HeaderType)theHeaderType;

	BaseMessage *aMsg = NULL;

	switch(theHeaderType) 
	{
		case CommonService:         
		case EncryptedService:      
		case FactoryServer:
		case AuthServer:             
		case DirServer:             
		case ParamServer:            
		case ChatServer:             
		case SIGSAuthServer:         
		case OverlordServer:
		case Auth1Login:             
		case Auth1LoginHL:           
		case Auth1PeerToPeer:
			aMsg = new TMessage; 
			break;
      
		case HeaderService0Message0:
		case HeaderEncryptedService:
		case HeaderWithTag:
		case HeaderWithTagAndExpiration:
			aMsg = new HeaderMessage; 
			break;

		case HeaderService1Message1: // non-encrypted MiniMessages
		case MiniEncryptedService:   // encrypted Service1Message1 (aka MiniMessage)
			aMsg = new MiniMessage; 
			break;

		case HeaderService2Message2: // non-encrypted SmallMessages
		case SmallEncryptedService:  // encrypted Service2Message2 (aka SmallMessage)
			aMsg = new SmallMessage; 
			break;

		case HeaderService4Message4: // non-encrypted LargeMessage
		case LargeEncryptedService:  // encrypted Service4Message4 (aka LargeMessage)
			aMsg = new LargeMessage; 
			break;

		default:
			aMsg = NULL;
	}	

	if(aMsg!=NULL)
		aMsg->ResetWritePointer();

	return aMsg;
}

// HeaderMessage implementation ///////////////////////////////////////
HeaderMessage::HeaderMessage() :
	BaseMessage(eHeaderMessage)
{
	AllocateHeaderBuffer();
	SetHeaderType(HeaderService0Message0);
}

HeaderMessage::HeaderMessage(const HeaderMessage& msg) :
	BaseMessage(msg)
{}

HeaderMessage::HeaderMessage(unsigned long theDataLen, const void *theData) :
	BaseMessage(theDataLen, theData)
{
	mMessageClass = eHeaderMessage;
}

HeaderMessage::~HeaderMessage()
{}

HeaderMessage& HeaderMessage::operator=(const HeaderMessage& theMsgR)
{
	BaseMessage::operator=(theMsgR);
	return *this;
}

TRawMsg* HeaderMessage::Duplicate() const
{
	return new HeaderMessage(*this);
}

void* HeaderMessage::Pack()
{
// Don't pack the message since this is simply a wrapper for an underlying message which
// is already packed.
	return GetDataPtr();
}

void HeaderMessage::Dump(std::ostream& os) const
{
	os << "HeaderMessage Dump:" << endl;
	os << "  Data Size: " << GetDataLen() << endl;
	os << "  HeaderType: " << (int)GetHeaderType() << endl;
}

// MiniMessage implementation ///////////////////////////////////////
MiniMessage::MiniMessage() :
	BaseMessage(eMiniMessage)
{
	AllocateHeaderBuffer();
	SetHeaderType(HeaderService1Message1);
	SetServiceType(0);
	SetMessageType(0);
}

MiniMessage::MiniMessage(const MiniMessage& msg) :
	BaseMessage(msg)
{}

MiniMessage::MiniMessage(unsigned long theDataLen, const void *theData) :
	BaseMessage(theDataLen, theData)
{
	mMessageClass = eMiniMessage;
}

MiniMessage::~MiniMessage()
{}

MiniMessage& MiniMessage::operator=(const MiniMessage& theMsgR)
{
	BaseMessage::operator=(theMsgR);
	return *this;
}

TRawMsg* MiniMessage::Duplicate() const
{
	return new MiniMessage(*this);
}

void* MiniMessage::Pack()
{
	BaseMessage::Pack();
	return GetDataPtr();
}

void MiniMessage::Dump(std::ostream& os) const
{
	os << "MiniMessage Dump:" << endl;
	os << "  Data Size: " << GetDataLen() << endl;
	os << "  HeaderType: " << (int)GetHeaderType() << endl;
	os << "  ServiceType: " << (int)GetServiceType() << endl;
	os << "  MessageType: " << (int)GetMessageType() << endl;
}


// SmallMessage implementation ///////////////////////////////////////
SmallMessage::SmallMessage() :
	BaseMessage(eSmallMessage)
{
	AllocateHeaderBuffer();
	SetHeaderType(HeaderService2Message2);
	SetServiceType(0);
	SetMessageType(0);
}

SmallMessage::SmallMessage(const SmallMessage& msg) :
	BaseMessage(msg)
{}

SmallMessage::SmallMessage(unsigned long theDataLen, const void *theData) :
	BaseMessage(theDataLen, theData)
{
	mMessageClass = eSmallMessage;
}

SmallMessage::~SmallMessage()
{}

SmallMessage& SmallMessage::operator=(const SmallMessage& theMsgR)
{
	BaseMessage::operator=(theMsgR);
	return *this;
}

TRawMsg* SmallMessage::Duplicate() const
{
	return new SmallMessage(*this);
}

void* SmallMessage::Pack()
{
	BaseMessage::Pack();
	return GetDataPtr();
}

void SmallMessage::Dump(std::ostream& os) const
{
	os << "SmallMessage Dump:" << endl;
	os << "  Data Size: " << GetDataLen() << endl;
	os << "  HeaderType: " << (int)GetHeaderType() << endl;
	os << "  ServiceType: " << (int)GetServiceType() << endl;
	os << "  MessageType: " << (int)GetMessageType() << endl;
}


// LargeMessage implementation ///////////////////////////////////////
LargeMessage::LargeMessage() :
	BaseMessage(eLargeMessage)
{
	AllocateHeaderBuffer();
	SetHeaderType(HeaderService4Message4);
	SetServiceType(0);
	SetMessageType(0);
}

LargeMessage::LargeMessage(const LargeMessage& msg) :
	BaseMessage(msg)
{}

LargeMessage::LargeMessage(unsigned long theDataLen, const void *theData) :
	BaseMessage(theDataLen, theData)
{
	mMessageClass = eLargeMessage;
}

LargeMessage::~LargeMessage()
{}

LargeMessage& LargeMessage::operator=(const LargeMessage& theMsgR)
{
	BaseMessage::operator=(theMsgR);
	return *this;
}

TRawMsg* LargeMessage::Duplicate() const
{
	return new LargeMessage(*this);
}

void* LargeMessage::Pack()
{
	BaseMessage::Pack();
	return GetDataPtr();
}

void LargeMessage::Dump(std::ostream& os) const
{
	os << "LargeMessage Dump:" << endl;
	os << "  Data Size: " << GetDataLen() << endl;
	os << "  HeaderType: " << (int)GetHeaderType() << endl;
	os << "  ServiceType: " << (int)GetServiceType() << endl;
	os << "  MessageType: " << (int)GetMessageType() << endl;
}


// TMessage implementation ////////////////////////////////////////
// ** TMessages are OBSOLETE **
TMessage::TMessage() :
	BaseMessage(eTMessage)
{
	AllocateHeaderBuffer();
	SetServiceType(0);
	SetMessageType(0);
}

TMessage::TMessage(const TMessage& Tmsg) :
	BaseMessage(Tmsg)
{}

TMessage::TMessage(unsigned long theDataLen, const void *theData) :
	BaseMessage(theDataLen, theData)
{
	mMessageClass = eTMessage;
}

TMessage::~TMessage()
{}

TMessage& TMessage::operator= (const TMessage& theMsgR)
{
	BaseMessage::operator=(theMsgR);
	return *this;
}

TRawMsg* TMessage::Duplicate() const
{
	return new TMessage(*this);
}

void * TMessage::Pack()
{
	BaseMessage::Pack();

	return GetDataPtr();
}

void TMessage::Dump(std::ostream& os) const
{
	os << "TMessage Dump:" << endl;
	os << "  Data Size: " << GetDataLen() << endl;
	os << "  ServiceType: " << (int)GetServiceType() << endl;
	os << "  MessageType: " << (int)GetMessageType() << endl;
}
