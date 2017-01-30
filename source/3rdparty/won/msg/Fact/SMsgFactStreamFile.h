#ifndef SMSGFACTSTREAMFILE_H
#define SMSGFACTSTREAMFILE_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>

namespace WONMsg {

//
// SMsgFactStreamFile
//
class SMsgFactStreamFile : public SmallMessage {
public:
	// Default ctor
	SMsgFactStreamFile(void);

	// Unpacking ctor
	SMsgFactStreamFile(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactStreamFile(const SMsgFactStreamFile& theMsgR);

	// Destructor
	virtual ~SMsgFactStreamFile(void);

	// Assignment
	SMsgFactStreamFile& operator=(const SMsgFactStreamFile& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactStreamFile(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
	const std::string& GetFilePath() const { return mFilePath; }
	void SetFilePath(const std::string& theFilePath) { mFilePath = theFilePath; }

	bool GetCompress() const { return mCompress; }
	void SetCompress(bool theVal) { mCompress = theVal; }

	const FILETIME& GetModifyTime() const { return mModifyTime; }
	void SetModifyTime(const FILETIME& theVal) { mModifyTime = theVal; }

protected:
	std::string mFilePath;
	bool mCompress;
	FILETIME mModifyTime;
};

}; // namespace WONMsg

#endif // SMSGFACTStreamFile_H
