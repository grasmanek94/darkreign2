#ifndef SMSGFACTPULLCONFIGFILE_H
#define SMSGFACTPULLCONFIGFILE_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>

namespace WONMsg {

//
// SMsgFactPullConfigFile
//
class SMsgFactPullConfigFile : public SmallMessage {
public:
	// Default ctor
	SMsgFactPullConfigFile(void);

	// Unpacking ctor
	SMsgFactPullConfigFile(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactPullConfigFile(const SMsgFactPullConfigFile& theMsgR);

	// Destructor
	virtual ~SMsgFactPullConfigFile(void);

	// Assignment
	SMsgFactPullConfigFile& operator=(const SMsgFactPullConfigFile& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactPullConfigFile(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
	const std::string& GetConfigName() const { return mConfigName; }
	void SetConfigName(const std::string& theConfigName) { mConfigName = theConfigName; }
	const std::string& GetFilePath() const { return mFilePath; }
	void SetFilePath(const std::string& theFilePath) { mFilePath = theFilePath; }

protected:
	std::string mConfigName;
	std::string mFilePath;
};

}; // namespace WONMsg

#endif 