#ifndef _TMsgAuth1LoginReplyHL_H
#define _TMsgAuth1LoginReplyHL_H

// TMsgAuth1LoginReplyHL.h

// AuthServer login reply for Auth1 Half-Life login protocols.  This is the final
// message of the login exchange that delivers the Auth Certificate or failure
// information.  It may optionally deliver a new Auth Public Key Block, Client
// Private Key, secret confirmation, and/or extended error information

// This message contains an array of data.  The array may be composed as follows:
//	1) 0 or 1 Auth Certificate
//	2) 0 or 1 Client PrivateKey   (Encrypted with session key)
//	3) 0 or 1 Auth PublicKey Block
//	4) 0 or 1 Secret confirmation (Encrypted with session key)
//	5) 0 or more error strings.

// All items if present will be in the above order.  (1), (2), and (4) will only be
// present if status implies success.  (5) will only be present if status implies
// failure.  (3) may be present in all cases.

// The Secret confirmation contains:
//     2 byte random pad
//     Session key
// If the Session key in the message matches the
// session key sent in the Auth1Login message, the
// client knows that the data it received came from
// the auth server, and is not simply garbage.

// The Display Info contains:
//     1 byte flag
//     Nickname
// If the 1 byte flag is true, The NicknameKey in the
// Auth1LoginRequest was not found, and the Nickname
// is the default value.


#include "common/won.h"
#include <list>
#include <map>
#include "msg/TMessage.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgAuth1LoginReplyHL : public TMessage
{
public:
	// Types
	enum EntryType {
		LRCertificate      = 1,  // Certificate
		LRClientPrivateKey = 2,  // Client's Private Key
		LRPublicKeyBlock   = 3,  // AuthServer Public Key Block
		LRErrorInfo        = 4,  // Extended error info
		LRSecretConfirm    = 5,  // Client secret confirmation (encrypted)
	};

	typedef std::pair<const unsigned char*, unsigned short> RawBlock;
	typedef std::list<std::string> ErrorList;

	// Default ctor
	TMsgAuth1LoginReplyHL(ServiceType theServType);

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1LoginReplyHL(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1LoginReplyHL(const TMsgAuth1LoginReplyHL& theMsgR);

	// Destructor
	~TMsgAuth1LoginReplyHL();

	// Assignment
	TMsgAuth1LoginReplyHL& operator=(const TMsgAuth1LoginReplyHL& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	// Status access
	ServerStatus GetStatus() const;
	void         SetStatus(ServerStatus theStatus);

	// Error List access
	const ErrorList& ErrList() const;
	ErrorList&       ErrList();

	// Block access
	const RawBlock& GetRawBlock(EntryType theType) const;// May be null

	// Update a raw block.  Setting copyBlock to false will cause the specified raw
	// pointer to be stored without copying its contents.  This will improve
	// performance, but raw pointer MUST NOT BE DEALLOCATED while in use by this class.
	void SetRawBlock(EntryType theType, const unsigned char* theBlockP,
	                 unsigned short theLen, bool copyBlock=false);

	// Force copy of raw blocks if needed.
	void ForceOwn(EntryType theType);
	void ForceOwnAll();  // Do all blocks

private:
	// Types
	typedef std::map<EntryType, RawBlock> RawBlockMap;
	typedef std::map<EntryType, WONCommon::RawBuffer> BufferMap;

	// Members
	ServerStatus mStatus;   // Status of the request
	ErrorList    mErrList;  // List of extended error info (may be empty)
	RawBlockMap  mRawMap;   // Map of raw blocks
	BufferMap    mBufMap;   // Map of buffers for ownership of raw block data

	// Get a ref to block of specified type.  Inits block to (NULL,0) if needed.
	RawBlock& GetBlockRef(EntryType theType);

	// Copy blocks from another instance
	void CopyBlocks(const TMsgAuth1LoginReplyHL& theMsgR);

	// Pack/Unpack a block
	void PackBlock  (EntryType theType, RawBlock& theBlockR);
	void UnpackBlock(EntryType theType);
};


// Inlines
inline TRawMsg*
TMsgAuth1LoginReplyHL::Duplicate(void) const
{ return new TMsgAuth1LoginReplyHL(*this); }

inline ServerStatus
TMsgAuth1LoginReplyHL::GetStatus() const
{ return mStatus; }

inline void
TMsgAuth1LoginReplyHL::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const TMsgAuth1LoginReplyHL::ErrorList&
TMsgAuth1LoginReplyHL::ErrList() const
{ return mErrList; }

inline TMsgAuth1LoginReplyHL::ErrorList&
TMsgAuth1LoginReplyHL::ErrList()
{ return mErrList; }

inline const TMsgAuth1LoginReplyHL::RawBlock&
TMsgAuth1LoginReplyHL::GetRawBlock(EntryType theType) const
{ return const_cast<TMsgAuth1LoginReplyHL*>(this)->GetBlockRef(theType); }

};  // Namespace WONMsg

#endif