#include "common/won.h"
#include "common/OutputOperators.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "AddresseeList.h"

namespace {
	using WONMsg::AddresseeList;
};

AddresseeList::AddresseeList(void) : 
	mIncludeExcludeFlag(true)
{}

AddresseeList::~AddresseeList(void)
{}

AddresseeList::AddresseeList(const AddresseeList& theCopyR) :
	mIncludeExcludeFlag(theCopyR.mIncludeExcludeFlag),
	mAddresseeList(theCopyR.mAddresseeList)
{}

AddresseeList& AddresseeList::operator =(const AddresseeList& theCopyR)
{
	if (this != &theCopyR)
	{
		mIncludeExcludeFlag = theCopyR.mIncludeExcludeFlag;
		mAddresseeList      = theCopyR.mAddresseeList;
	}

	return *this;
}

void AddresseeList::Dump(ostream& os) const
{
	os << "  Include/Exclude: " << (mIncludeExcludeFlag ? "include" : "exclude") << mAddresseeList << endl;
}

void AddresseeList::AppendAddresseeList(RoutingServerMessage* theMsgP)
{
	WTRACE("AddresseeList::AppendAddresseeList");

	// append the list of client/group ids
	IdList::iterator itr = mAddresseeList.begin();
	while (itr != mAddresseeList.end())
		theMsgP->AppendClientOrGroupId(*(itr++));
}

void AddresseeList::ReadAddresseeList(RoutingServerMessage* theMsgP)
{
	WTRACE("AddresseeList::ReadAddresseeList");

	for (unsigned long aBytesLeftToRead = theMsgP->BytesLeftToRead(); aBytesLeftToRead > 0; aBytesLeftToRead -= 2)
	{
		if (aBytesLeftToRead == 1)
			throw WONMsg::BadMsgException(*theMsgP, __LINE__, __FILE__, "Odd number of bytes passed as Addressee List!");

		// add the addressee to the list
		mAddresseeList.push_back(theMsgP->ReadClientOrGroupId());
	}
}