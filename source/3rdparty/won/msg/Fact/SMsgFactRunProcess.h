#ifndef SMSGFACTRUNPROCESS_H
#define SMSGFACTRUNPROCESS_H

#include "msg/TMessage.h"

namespace WONMsg {

//
// SMsgFactRunProcess
//
class SMsgFactRunProcess : public SmallMessage {
public:
	// Default ctor
	SMsgFactRunProcess(void);

	// Unpacking ctor
	SMsgFactRunProcess(const SmallMessage& theMsgR, bool doUnpack =true);

	// Copy ctor
	SMsgFactRunProcess(const SMsgFactRunProcess& theMsgR);

	// Destructor
	virtual ~SMsgFactRunProcess(void);

	// Assignment
	SMsgFactRunProcess& operator=(const SMsgFactRunProcess& theMsgR);

	// Duplicate
	virtual TRawMsg* Duplicate(void) const { return new SMsgFactRunProcess(*this); }

	// Debug output
	virtual void Dump(std::ostream& os) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type.
	virtual void* Pack(void);
	virtual void  Unpack(void);

	// Member access
	const std::string& GetCommandLine() const { return mCommandLine; }
	void SetCommandLine(const std::string& theCommandLine) { mCommandLine = theCommandLine; }

	const std::string& GetWorkingDirectory() const { return mWorkingDirectory; }
	void SetWorkingDirectory(const std::string& theWorkingDirectory) { mWorkingDirectory = theWorkingDirectory; }

	bool GetRunDetached() const { return mRunDetached; }
	void SetRunDetached(bool theVal) { mRunDetached = theVal; }

protected:
	std::string mCommandLine;
	std::string mWorkingDirectory;
	bool mRunDetached;
};

}; // namespace WONMsg

#endif // SMSGFACTRUNPROCESS_H
