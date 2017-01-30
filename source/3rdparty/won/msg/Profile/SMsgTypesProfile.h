#ifndef __SMSGTYPESPROFILE_H__
#define __SMSGTYPESPROFILE_H__


namespace WONMsg
{
	enum MsgTypeProfile
	{
		ProfileCreateRequest = 0,
		ProfileCreateResponse,

		ProfileUpdateRequest,
		ProfileUpdateResponse,

		ProfileReadRequest,
		ProfileReadResponse,

		ProfileCheckFieldsRequest,
		ProfileCheckFieldsResponse,

		// new msgs
		Profile2Get,
		Profile2GetReply,

		Profile2Set,
		Profile2SetReply,

		Profile2GetCommunities,
		Profile2GetCommunitiesReply,

		Profile2GetNewsletters,
		Profile2GetNewslettersReply,

		Profile2GetCommunityNewsletters,
		Profile2GetCommunityNewslettersReply,

		Profile2SetNewsletters,
		Profile2SetNewslettersReply,

		Profile2DownloadCommunityTree,
		Profile2DownloadCommunityTreeReply,

		// Last Message type.  Don't use
		ProfileMsgMax
	};
};


#endif // SMsgTypesContest_H
