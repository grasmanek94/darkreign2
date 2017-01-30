#ifndef SMSGFACTSAVECONFIGFILE_H
#define SMSGFACTSAVECONFIGFILE_H

#include "msg/TMessage.h"
#include "common/won.h"
#include <string>

namespace WONMsg {

//
// SMsgFactSaveConfigFile
//
class SMsgFactSaveConfigFile : public SmallMessage {
public:
	// Default ctor
	SMsgFactSaveConfigFile(void);

	// Unpacking ctor
	SMsgFactSaveConfigFile(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactSaveConfigFile(const SMsgFactSaveConfigFile& theMsgR);

	// Destructor
	virtual ~SMsgFactSaveConfigFile(void);

	// Assignment
	SMsgFactSaveConfigFile& operator=(const SMsgFactSaveConfigFile& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactSaveConfigFile(*this); }

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
	const WONCommon::RawBuffer& GetBlob() const { return mBlob; }
	void SetBlob(const WONCommon::RawBuffer& theBlob) { mBlob = theBlob; }
protected:
	std::string mConfigName;
	std::string mFilePath;
	WONCommon::RawBuffer mBlob;
};

}; // namespace WONMsg

#endif // SMSGFACTSAVECONFIGFILE_H
