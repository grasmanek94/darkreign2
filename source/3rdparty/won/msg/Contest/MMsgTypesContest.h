#ifndef __MMSGTYPESCONTEST_H__
#define __MMSGTYPESCONTEST_H__


namespace WONMsg
{
	enum MsgTypeContest
	{
		// First message type.  Don't use
		ContestMsgMin = 0,

		// Event Server Messages
		ContestDBCall	       = ContestMsgMin + 1,
		ContestDBCallReply     = ContestMsgMin + 2,

		// Last Message type.  Don't use
		ContestMsgMax
	};

};


#endif // MMsgTypesContest_H
