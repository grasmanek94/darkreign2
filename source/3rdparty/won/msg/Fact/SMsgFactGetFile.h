#ifndef SMSGFactGetFile_H
#define SMSGFactGetFile_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>

namespace WONMsg {

//
// SMsgFactGetFile
//
class SMsgFactGetFile : public SmallMessage {
public:
	// Default ctor
	SMsgFactGetFile(void);

	// Unpacking ctor
	SMsgFactGetFile(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactGetFile(const SMsgFactGetFile& theMsgR);

	// Destructor
	virtual ~SMsgFactGetFile(void);

	// Assignment
	SMsgFactGetFile& operator=(const SMsgFactGetFile& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactGetFile(*this); }

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

	unsigned long GetBlockLen() const { return mBlockLen; }
	void SetBlockLen(unsigned long theVal) { mBlockLen = theVal; }

protected:
	std::string mFilePath;
	bool mCompress;
	unsigned long mBlockLen;
};

}; // namespace WONMsg

#endif // SMSGFactGetFile_H
