// SMsgDirG2SetACLs.h

// DirectoryServer set ACLs message.  Updates the ACLs attached to an entity.
// This message can be used to any combinations of add, modify, or remove
// of permissions within all types of ACLs on an entity.


#include "common/won.h"
#include "auth/Permission.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2SetACLs.h"

// Private namespace for using, types, and constants
namespace {
	using WONAuth::Permission;
	using WONAuth::PermissionACL;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateBase;
	using WONMsg::SMsgDirG2SetACLs;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2SetACLs::SMsgDirG2SetACLs(KeyType theType) :
	SMsgDirG2UpdateBase(theType),
	mACLs()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceSetACLs : WONMsg::DirG2DirectorySetACLs);
}


// SmallMessage ctor
SMsgDirG2SetACLs::SMsgDirG2SetACLs(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mACLs()
{
	Unpack();
}


// Copy ctor
SMsgDirG2SetACLs::SMsgDirG2SetACLs(const SMsgDirG2SetACLs& theMsgR) :
	SMsgDirG2UpdateBase(theMsgR),
	mACLs(theMsgR.mACLs)
{}


// Destructor
SMsgDirG2SetACLs::~SMsgDirG2SetACLs(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2SetACLs&
SMsgDirG2SetACLs::operator=(const SMsgDirG2SetACLs& theMsgR)
{
	SMsgDirG2UpdateBase::operator=(theMsgR);
	mACLs = theMsgR.mACLs;
	return *this;
}


// SMsgDirG2SetDataObjects::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2SetACLs::Pack(void)
{
	WTRACE("SMsgDirG2SetACLs::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mKeyType == KT_SERVICE ? WONMsg::DirG2ServiceSetACLs : WONMsg::DirG2DirectorySetACLs);
	SMsgDirG2UpdateBase::Pack();

	PackKey(*this);

	WDBG_LL("SMsgDirG2SetACLs::Pack Writing " << mACLs.size() << " ACLs.");
	AppendShort(mACLs.size());

	DirUpdateACLList::const_iterator anACLItr(mACLs.begin());
	for (; anACLItr != mACLs.end(); anACLItr++)
	{
		AppendByte(anACLItr->mMode);
		AppendByte(anACLItr->mDirACL.mType);
		AppendShort(anACLItr->mDirACL.mACL.size());
		PermissionACL::const_iterator aPermItr(anACLItr->mDirACL.mACL.begin());
		for (; aPermItr != anACLItr->mDirACL.mACL.end(); aPermItr++)
		{
			AppendLong(aPermItr->mUserId);
			AppendLong(aPermItr->mCommunityId);
			AppendShort(aPermItr->mTrustLevel);
		}
	}

	PackPeerData();
	return GetDataPtr();
}


// SMsgDirG2SetDataObjects::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2SetACLs::Unpack(void)
{
	WTRACE("SMsgDirG2SetACLs::Unpack");
	mKeyType = (GetMessageType() == WONMsg::DirG2ServiceSetACLs ? KT_SERVICE : KT_DIRECTORY);
	SMsgDirG2UpdateBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2DirectorySetACLs) &&
	     (GetMessageType() != WONMsg::DirG2ServiceSetACLs)))
	{
		WDBG_AH("SMsgDirG2SetACLs::Unpack Not a DirG2SetACLs message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2SetACLs message.");
	}

	UnpackKey(*this);

	unsigned short aCt = ReadShort();
	WDBG_LL("SMsgDirG2SetACLs::Unpack Reading " << aCt << " ACLs.");

	for (int i=0; i < aCt; i++)
	{
		DirUpdateACL anACL;
		anACL.mMode         = static_cast<WONMsg::DirG2ACLSetMode>(ReadByte());
		anACL.mDirACL.mType = static_cast<WONMsg::DirG2ACLType>(ReadByte());

		unsigned short aPermCt = ReadShort();
		for (int j=0; j < aPermCt; j++)
		{
			Permission aPerm;
			aPerm.mUserId      = ReadLong();
			aPerm.mCommunityId = ReadLong();
			aPerm.mTrustLevel  = ReadShort();
			anACL.mDirACL.mACL.insert(aPerm);
		}

		mACLs.push_back(anACL);
	}

	UnpackPeerData();
}
