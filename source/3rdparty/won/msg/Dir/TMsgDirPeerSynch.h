#ifndef _TMsgDirPeerSynch_H
#define _TMsgDirPeerSynch_H

// TMsgDirPeerSynch.h

// Directory Get Directory Contents Message class.  Fetches contents of a
// Directory from the Directory Server.


#include <string>
#include "msg/TMessage.h"
#include "DirServerEntry.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirPeerSynch : public TMessage
{
public:
	// Default ctor
	TMsgDirPeerSynch(void);

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgDirPeerSynch(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirPeerSynch(const TMsgDirPeerSynch& theMsgR);

	// Destructor
	~TMsgDirPeerSynch(void);

	// Assignment
	TMsgDirPeerSynch& operator=(const TMsgDirPeerSynch& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Server ID access
	const std::string& GetServId() const;
	void               SetServId(const std::string& theId);

	// Entry access (const and non-const versions)
	const DirServerEntryVector& Entries() const;
	DirServerEntryVector&       Entries();

	// Get base size of messsage
	unsigned long GetBaseSize() const;

private:
	std::string          mServId;   // Server ID
	DirServerEntryVector mEntries;  // Entries returned (may be empty)

	// Private methods
	void PackEntry(const DirServerEntry& theEntry);
	void UnpackEntry(DirServerEntry& theEntry);
};


// Inlines
inline TRawMsg*
TMsgDirPeerSynch::Duplicate(void) const
{ return new TMsgDirPeerSynch(*this); }

inline const std::string&
TMsgDirPeerSynch::GetServId() const
{ return mServId; }

inline void
TMsgDirPeerSynch::SetServId(const std::string& theId)
{ mServId = theId; }

inline const DirServerEntryVector&
TMsgDirPeerSynch::Entries() const
{ return mEntries; }

inline DirServerEntryVector&
TMsgDirPeerSynch::Entries()
{ return mEntries; }


};  // Namespace WONMsg

#endif