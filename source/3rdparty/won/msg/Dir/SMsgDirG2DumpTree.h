#ifndef _SMsgDirG2DumpTree_H
#define _SMsgDirG2DumpTree_H

// SMsgDirG2DumpTree.h

// Directory Dump Tree Message class.  Outputs the Directory Server internal
// directory/service tree to a specified file (path).  Message specifies the
// file name to output the tree to and an option path to a directory to write
// the output file in.  If the directory is NULL, file will be output in the
// "C:\temp" directory.


#include <string>
#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgDirG2DumpTree : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2DumpTree(void);

	// SmallMessage ctor - will throw if SmallMessage type DirG2DumpTree
	explicit SMsgDirG2DumpTree(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2DumpTree(const SMsgDirG2DumpTree& theMsgR);

	// Destructor
	~SMsgDirG2DumpTree(void);

	// Assignment
	SMsgDirG2DumpTree& operator=(const SMsgDirG2DumpTree& theMsgR);

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
SMsgDirG2DumpTree::Duplicate(void) const
{ return new SMsgDirG2DumpTree(*this); }

inline const std::string&
SMsgDirG2DumpTree::GetFilePath(void) const
{ return mFilePath; }

inline void
SMsgDirG2DumpTree::SetFilePath(const std::string& thePath)
{ mFilePath = thePath; }

inline const std::string&
SMsgDirG2DumpTree::GetFileName(void) const
{ return mFileName; }

inline void
SMsgDirG2DumpTree::SetFileName(const std::string& theName)
{ mFileName = theName; }


};  // Namespace WONMsg

#endif