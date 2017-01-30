#ifndef _TMsgDirDumpTree_H
#define _TMsgDirDumpTree_H

// TMsgDirDumpTree.h

// Directory Dump Tree Message class.  Outputs the Directory Server internal
// directory/service tree to a specified file (path).  Message specifies the
// file name to output the tree to and an option path to a directory to write
// the output file in.  If the directory is NULL, file will be output in the
// "C:\temp" directory.


#include <string>
#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirDumpTree : public TMessage
{
public:
	// Default ctor
	TMsgDirDumpTree(void);

	// TMessage ctor - will throw if TMessage type is not CommDebugLevel
	explicit TMsgDirDumpTree(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirDumpTree(const TMsgDirDumpTree& theMsgR);

	// Destructor
	~TMsgDirDumpTree(void);

	// Assignment
	TMsgDirDumpTree& operator=(const TMsgDirDumpTree& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Path access (may be NULL)
	const std::string& GetFilePath(void) const;
	void               SetFilePath(const std::string& thePath);

	// File name access
	const std::string& GetFileName(void) const;
	void               SetFileName(const std::string& theName);

private:
	std::string mFilePath;  // Path to directory to write file (may be NULL)
	std::string mFileName;  // Name of file to dump tree to
};


// Inlines
inline TRawMsg*
TMsgDirDumpTree::Duplicate(void) const
{ return new TMsgDirDumpTree(*this); }

inline const std::string&
TMsgDirDumpTree::GetFilePath(void) const
{ return mFilePath; }

inline void
TMsgDirDumpTree::SetFilePath(const std::string& thePath)
{ mFilePath = thePath; }

inline const std::string&
TMsgDirDumpTree::GetFileName(void) const
{ return mFileName; }

inline void
TMsgDirDumpTree::SetFileName(const std::string& theName)
{ mFileName = theName; }


};  // Namespace WONMsg

#endif