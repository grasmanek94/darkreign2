#ifndef SMsgFactGetDirReply_H
#define SMsgFactGetDirReply_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>
#include <list>

namespace WONMsg {

//
// SMsgFactGetDirReply
//
class SMsgFactGetDirReply : public SmallMessage {
public:
	struct FileEntry
	{
		std::string mName;
		DWORD mAttributes;
		FILETIME mModifyTime;
		__int64 mFileSize;	

		FileEntry() { mAttributes = 0; memset(&mModifyTime,0,sizeof(FILETIME)); mFileSize = 0; }
		FileEntry(const std::string &theName, DWORD theAttributes, FILETIME theModifyTime, __int64 theFileSize) :
			mName(theName), mAttributes(theAttributes), mModifyTime(theModifyTime), mFileSize(theFileSize) { }
	};

	typedef std::list<FileEntry> FileList;


public:
	// Default ctor
	SMsgFactGetDirReply(void);

	// Unpacking ctor
	SMsgFactGetDirReply(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactGetDirReply(const SMsgFactGetDirReply& theMsgR);

	// Destructor
	virtual ~SMsgFactGetDirReply(void);

	// Assignment
	SMsgFactGetDirReply& operator=(const SMsgFactGetDirReply& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactGetDirReply(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
	short GetStatus() const { return mStatus; }
	void SetStatus(short theVal) { mStatus = theVal; }

	FileList& GetFileList() { return mFileList; }

protected:
	short mStatus;
	FileList mFileList;

};

}; // namespace WONMsg

#endif // SMsgFactGetDirReply_H
