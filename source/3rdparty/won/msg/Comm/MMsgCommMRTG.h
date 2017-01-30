#ifndef _MMsgCommMRTG_H
#define _MMsgCommMRTG_H

// MMsgCommMRTG.h

// Common MRTG message class.  Supports MRTG requests and replies to
// WON servers.  


#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgCommMRTGRequest : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommMRTGRequest(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommGetNetStat
	explicit MMsgCommMRTGRequest(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommMRTGRequest(const MMsgCommMRTGRequest& theMsgR);

	// Destructor
	~MMsgCommMRTGRequest(void);

	// Assignment
	MMsgCommMRTGRequest& operator=(const MMsgCommMRTGRequest& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const { return new MMsgCommMRTGRequest(*this); }

	// Pack and Unpack the message
	void* Pack(void); 
	void  Unpack(void);

	unsigned short GetPrimaryID() const { return mPrimaryID; }
	void SetPrimaryID(unsigned short thePrimaryID) { mPrimaryID = thePrimaryID; }

	unsigned short GetSecondaryID() const { return mSecondaryID; }
	void SetSecondaryID(unsigned short theSecondaryID) { mSecondaryID = theSecondaryID; }

	void GetPrimaryOption(WONCommon::RawBuffer& theOptionR) const { theOptionR = mPrimaryOption; }
	void SetPrimaryOption(const WONCommon::RawBuffer& thePrimaryOption) { mPrimaryOption = thePrimaryOption; }

	void GetSecondaryOption(WONCommon::RawBuffer& theOptionR) const { theOptionR = mSecondaryOption; }
	void SetSecondaryOption(const WONCommon::RawBuffer& theSecondaryOption) { mSecondaryOption = theSecondaryOption; }

private:
	unsigned short mPrimaryID;
	unsigned short mSecondaryID;

	WONCommon::RawBuffer mPrimaryOption;
	WONCommon::RawBuffer mSecondaryOption;
};


class MMsgCommMRTGReply : public MiniMessage
{
public:
	// Default ctor
	explicit MMsgCommMRTGReply(void);

	// MiniMessage ctor - will throw if MiniMessage type is not CommGetNetStatReply
	explicit MMsgCommMRTGReply(const MiniMessage& theMsgR);

	// Copy ctor
	MMsgCommMRTGReply(const MMsgCommMRTGReply& theMsgR);

	// Destructor
	~MMsgCommMRTGReply(void);

	// Assignment
	MMsgCommMRTGReply& operator=(const MMsgCommMRTGReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const { return new MMsgCommMRTGReply(*this); }

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message type is not CommDebugLevel
	void* Pack(void); 
	void  Unpack(void);

	short GetStatus() const { return mStatus; }
	void SetStatus(short theStatus) { mStatus = theStatus; }

	__int64 GetPrimaryData() const { return mPrimaryData; }
	void SetPrimaryData(__int64 thePrimaryData) { mPrimaryData = thePrimaryData; }

	__int64 GetSecondaryData() const { return mSecondaryData; }
	void SetSecondaryData(__int64 theSecondaryData) { mSecondaryData = theSecondaryData; }

private:
	short   mStatus;
	__int64 mPrimaryData;
	__int64 mSecondaryData;
};



};  // Namespace WONMsg

#endif