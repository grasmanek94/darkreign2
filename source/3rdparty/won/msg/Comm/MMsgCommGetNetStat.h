#ifndef _MMsgCommGetNetStat_H
#define _MMsgCommGetNetStat_H

// MMsgCommGetNetStat.h

// Common GetNetStat message class.  Supports GetNetStat requests and replys to
// WON servers.  GetNetStat reply may return extended info if requested.


#include "msg/TMessage.h"
#include <list>

// Forwards from WONSocket
namespace WONMsg {

class MMsgCommGetNetStat : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommGetNetStat(bool getExtended=false);

	// MiniMessage ctor - will throw if MiniMessage type is not CommGetNetStat
	explicit MMsgCommGetNetStat(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommGetNetStat(const MMsgCommGetNetStat& theMsgR);

	// Destructor
	~MMsgCommGetNetStat(void);

	// Assignment
	MMsgCommGetNetStat& operator=(const MMsgCommGetNetStat& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const { return new MMsgCommGetNetStat(*this); }

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);
};


class MMsgCommGetNetStatReply : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommGetNetStatReply(bool isExtended=false);

	// MiniMessage ctor - will throw if MiniMessage type is not CommGetNetStatReply
	explicit MMsgCommGetNetStatReply(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommGetNetStatReply(const MMsgCommGetNetStatReply& theMsgR);

	// Destructor
	~MMsgCommGetNetStatReply(void);

	// Assignment
	MMsgCommGetNetStatReply& operator=(const MMsgCommGetNetStatReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const { return new MMsgCommGetNetStatReply(*this); }

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	unsigned long GetNumSockets() const { return mNumSockets; }
	void SetNumSockets(unsigned long theNumSockets) { mNumSockets = theNumSockets; }

	std::list<unsigned short>& GetTCPListenSockets() { return mTCPListenSockets; }
	std::list<unsigned short>& GetUDPListenSockets() { return mUDPListenSockets; }
	std::list<DWORD>& GetLastAcceptTimes() { return mLastAcceptTimes; }

	DWORD mConnectToListenSocketCount;
	std::list<DWORD> mPendingAccepts;

private:
	unsigned long mNumSockets;

	std::list<unsigned short> mTCPListenSockets;
	std::list<unsigned short> mUDPListenSockets;
	std::list<DWORD> mLastAcceptTimes;
};



};  // Namespace WONMsg

#endif