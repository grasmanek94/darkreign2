#ifndef __MMSGTYPESDB_H__
#define __MMSGTYPESDB_H__


namespace WONMsg
{
	enum MsgTypeDB
	{
		// First message type.  Don't use
		DBMsgMin = 100,

		// Event Server Messages
		DBVerifyUser	       = DBMsgMin + 1,
		DBVerifyUserReply      = DBMsgMin + 2,
		DBRegisterUser		   = DBMsgMin + 3,
		DBRegisterUserReply    = DBMsgMin + 4,

		// Last Message type.  Don't use
		DBMsgMax
	};

};


#endif // MMsgTypesContest_H
