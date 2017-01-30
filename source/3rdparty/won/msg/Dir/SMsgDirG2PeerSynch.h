#ifndef _SMsgDirG2PeerSynch_H
#define _SMsgDirG2PeerSynch_H

// SMsgDirG2PeerSynch

// Peer synchronization message classes.  Defines classes to test a directory to
// see if a synch is needed (PeerSynchTest), request a directory be synchronized
// (PeerSynchPath), and synch a directoru (PeerSynch).  These messages interact
// as follows:
//		1) PeerSynchPath is sent in response to PeerSynchTest if synch is needed
//		2) PeerSynch is sent in response to PeerSynchPath


#include <string>
#include "msg/TMessage.h"
#include "SMsgDirG2EntityListBase.h"

// Forwards from WONSocket
namespace WONMsg {

// PeerSynchTest - Test a specified directory to see if synch is needed
class SMsgDirG2PeerSynchTest : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2PeerSynchTest(void);

	// SmallMessage ctor - will throw if SmallMessage type DirG2PeerSynchTest
	explicit SMsgDirG2PeerSynchTest(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerSynchTest(const SMsgDirG2PeerSynchTest& theMsgR);

	// Destructor
	~SMsgDirG2PeerSynchTest(void);

	// Assignment
	SMsgDirG2PeerSynchTest& operator=(const SMsgDirG2PeerSynchTest& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Server ID access
	const std::string& GetServId() const;
	void               SetServId(const std::string& theId);

	// Dir path access (may be NULL)
	const std::wstring& GetDirPath(void) const;
	void                SetDirPath(const std::wstring& thePath);

	// Dir CRC access
	unsigned long GetDirCRC(void) const;
	void          SetDirCRC(unsigned long theCRC);

	// Dir servicecount access
	unsigned short GetServCt(void) const;
	void           SetServCt(unsigned short theCt);

private:
	std::string    mServId;   // Server ID
	std::wstring   mDirPath;  // Directory path
	unsigned long  mDirCRC;   // Directory CRC
	unsigned short mServCt;   // Directory service count
};


// PeerSynchPath - request a specified directory be synchronized
class SMsgDirG2PeerSynchPath : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2PeerSynchPath(void);

	// SmallMessage ctor - will throw if SmallMessage type SMsgDirG2PeerSynchPath
	explicit SMsgDirG2PeerSynchPath(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerSynchPath(const SMsgDirG2PeerSynchPath& theMsgR);

	// Destructor
	~SMsgDirG2PeerSynchPath(void);

	// Assignment
	SMsgDirG2PeerSynchPath& operator=(const SMsgDirG2PeerSynchPath& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Server ID access
	const std::string& GetServId() const;
	void               SetServId(const std::string& theId);

	// Dir path access (may be NULL)
	const std::wstring& GetDirPath(void) const;
	void                SetDirPath(const std::wstring& thePath);

private:
	std::string  mServId;   // Server ID
	std::wstring mDirPath;  // Directory path
};


// PeerSynch - Synchronize a directory
class SMsgDirG2PeerSynch : public SMsgDirG2EntityListBase
{
public:
	// Default ctor
	SMsgDirG2PeerSynch(void);

	// SmallMessage ctor - will throw if SmallMessage type is not of this type
	explicit SMsgDirG2PeerSynch(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerSynch(const SMsgDirG2PeerSynch& theMsgR);

	// Destructor
	~SMsgDirG2PeerSynch(void);

	// Assignment
	SMsgDirG2PeerSynch& operator=(const SMsgDirG2PeerSynch& theMsgR);

	// Virtual Duplicate from SmallMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Server ID access
	const std::string& GetServId() const;
	void               SetServId(const std::string& theId);

	// Get current size of messsage (in bytes)
	unsigned long ComputePackSize() const;

	// Flags for pack/unpack entities (from SMsgDirg2EntityBase)
	unsigned long GetFlags() const;

private:
	std::string mServId;  // Server ID

	// Class constants
	static const unsigned long gGetFlags;  // Get flags for EntityBase
};


// Inlines
inline TRawMsg*
SMsgDirG2PeerSynchTest::Duplicate(void) const
{ return new SMsgDirG2PeerSynchTest(*this); }

inline const std::wstring&
SMsgDirG2PeerSynchTest::GetDirPath(void) const
{ return mDirPath; }

inline void
SMsgDirG2PeerSynchTest::SetDirPath(const std::wstring& thePath)
{ mDirPath = thePath; }

inline unsigned long
SMsgDirG2PeerSynchTest::GetDirCRC(void) const
{ return mDirCRC; }

inline void
SMsgDirG2PeerSynchTest::SetDirCRC(unsigned long theCRC)
{ mDirCRC = theCRC; }

inline unsigned short
SMsgDirG2PeerSynchTest::GetServCt(void) const
{ return mServCt; }

inline void
SMsgDirG2PeerSynchTest::SetServCt(unsigned short theCt)
{ mServCt = theCt; }

inline const std::string&
SMsgDirG2PeerSynchTest::GetServId() const
{ return mServId; }

inline void
SMsgDirG2PeerSynchTest::SetServId(const std::string& theId)
{ mServId = theId; }

inline TRawMsg*
SMsgDirG2PeerSynchPath::Duplicate(void) const
{ return new SMsgDirG2PeerSynchPath(*this); }

inline const std::wstring&
SMsgDirG2PeerSynchPath::GetDirPath(void) const
{ return mDirPath; }

inline void
SMsgDirG2PeerSynchPath::SetDirPath(const std::wstring& thePath)
{ mDirPath = thePath; }

inline const std::string&
SMsgDirG2PeerSynchPath::GetServId() const
{ return mServId; }

inline void
SMsgDirG2PeerSynchPath::SetServId(const std::string& theId)
{ mServId = theId; }

inline TRawMsg*
SMsgDirG2PeerSynch::Duplicate(void) const
{ return new SMsgDirG2PeerSynch(*this); }

inline const std::string&
SMsgDirG2PeerSynch::GetServId() const
{ return mServId; }

inline void
SMsgDirG2PeerSynch::SetServId(const std::string& theId)
{ mServId = theId; }

inline unsigned long
SMsgDirG2PeerSynch::GetFlags() const
{ return gGetFlags; }


};  // Namespace WONMsg

#endif