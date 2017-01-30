#ifndef PERMISSION_H
#define PERMISSION_H

// Permission

// Definitions for WON permissions and ACLs (Access Control Lists).  These objects
// are used to define access restrictions to various resources.

// Permissions are triples defining UserId, CommunityId, and TrustLevel.  These
// fields restrict access as follows:
//		UserId:       Must match if non-zero in permission
//		CommunityId:  Must match if non-zero in permission
//		TrustLevel:   Must be >= value in permission
// Permissions support a full set of comparison operators and are compared
// only on their UserId and CommunityId fields.


// Permission ACLs are a set of permissions that define the access rights
// for a resource.  PermissionACL contains a constructor to init from a
// a List of List[3] longs (list of list of 3 longs).  This is the format
// used in the framework options classes to define ACLs.


#include <vector>
#include <set>
#ifdef _LINUX
#include <ostream.h>
#else
#include <ostream>
#endif

namespace WONAuth {

// Permission
struct Permission
{
	unsigned long  mUserId;
	unsigned long  mCommunityId;
	unsigned short mTrustLevel;

	explicit Permission(unsigned long theUserId=0, unsigned long theCommunityId=0, unsigned short theTrustLevel=0) :
		mUserId(theUserId), mCommunityId(theCommunityId), mTrustLevel(theTrustLevel)
	{}

	int Compare(const Permission& thePermR) const
	{
		int aTst = mUserId - thePermR.mUserId;
		return (aTst == 0 ? mCommunityId - thePermR.mCommunityId : aTst);
	}

	bool operator==(const Permission& thePermR) const { return (Compare(thePermR) == 0); }
	bool operator!=(const Permission& thePermR) const { return (Compare(thePermR) != 0); }
	bool operator< (const Permission& thePermR) const { return (Compare(thePermR) <  0); }
	bool operator<=(const Permission& thePermR) const { return (Compare(thePermR) <= 0); }
	bool operator> (const Permission& thePermR) const { return (Compare(thePermR) >  0); }
	bool operator>=(const Permission& thePermR) const { return (Compare(thePermR) >= 0); }
};


// Access Control List
class PermissionACL : public std::set<Permission>
{
public:
	// List of List[3] of longs
	// This type allows inits from the BaseOptions::OptLongListList type and it's
	// definition MUST be equivalent.
	typedef std::vector<std::vector<long> > InitList;

	explicit PermissionACL(bool allowAdmin =true, bool emptyACLAllowsAnyone =true) : mAllowAdmins(allowAdmin), mEmptyACLAllowsAnyone(emptyACLAllowsAnyone) {}
	explicit PermissionACL(const InitList& theInitListR, bool allowAdmin =true, bool emptyACLAllowsAnyone =true) : 
		mAllowAdmins(allowAdmin), mEmptyACLAllowsAnyone(emptyACLAllowsAnyone)
	{
		for (InitList::const_iterator anItr = theInitListR.begin(); anItr != theInitListR.end(); ++anItr)
		{
			if (anItr->size() >= 3)
				insert(Permission((*anItr)[0], (*anItr)[1], (*anItr)[2]));
		}
	}

	bool mAllowAdmins;
	bool mEmptyACLAllowsAnyone;
};
	

}; // namespace WONAuth


// Output operators
inline std::ostream&
operator<<(std::ostream& os, const WONAuth::Permission& thePermR)
{
	os << '(' << thePermR.mUserId << ',' << thePermR.mCommunityId << ',' << thePermR.mTrustLevel << ')';
	return os;
}


inline std::ostream&
operator<<(std::ostream& os, const WONAuth::PermissionACL& theACLR)
{
	os << '[';
	if (! theACLR.empty())
	{
		WONAuth::PermissionACL::const_iterator anItr(theACLR.begin());
		os << *(anItr++);
		while (anItr != theACLR.end())
			os << ' ' << *(anItr++);
	}
	os << ']';
	return os;
}


#endif // PERMISSION_H
