#ifndef _DirACLs_H
#define _DirACLs_H

// DirACLs

// Defines a objects for managing Directory Server ACLs within messages.


#include <list>
#include "auth/Permission.h"
#include "DirG2Flags.h"

// In the WONMsg namespace
namespace WONMsg {

// DirACL - defines ACLs used in dir query request
struct DirACL
{
	DirG2ACLType           mType;
	WONAuth::PermissionACL mACL;

	DirACL() : mType(DAT_UNKNOWN), mACL() {}
};

// Define list of DirACLs
typedef std::list<DirACL> DirACLList;

// DirUpdateACL - defines ACLs used is dir update requests
struct DirUpdateACL
{
	DirG2ACLSetMode mMode;
	DirACL          mDirACL;

	DirUpdateACL() : mMode(DASM_ADDREPLACE), mDirACL() {}
};

// Define list of DirUpdateACLs
typedef std::list<DirUpdateACL> DirUpdateACLList;



};  // Namespace WONMsg

#endif