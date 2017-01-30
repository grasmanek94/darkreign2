#ifndef SMsgFactGetDir_H
#define SMsgFactGetDir_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>

namespace WONMsg {

//
// SMsgFactGetDir
//
class SMsgFactGetDir : public SmallMessage {
public:
	// Default ctor
	SMsgFactGetDir(void);

	// Unpacking ctor
	SMsgFactGetDir(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactGetDir(const SMsgFactGetDir& theMsgR);

	// Destructor
	virtual ~SMsgFactGetDir(void);

	// Assignment
	SMsgFactGetDir& operator=(const SMsgFactGetDir& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactGetDir(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
	const std::string& GetFilePath() const { return mFilePath; }
	void SetFilePath(const std::string& theFilePath) { mFilePath = theFilePath; }

protected:
	std::string mFilePath;
};

}; // namespace WONMsg

#endif // SMsgFactGetDir_H
