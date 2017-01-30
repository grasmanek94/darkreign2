#ifndef _MMsgTypesObs_H_
#define _MMsgTypesObs_H_

//
// Titan Observation Server messages
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// Never change an existing enum value and always add new values to
// the end.

namespace WONMsg
{
	enum MsgTypeObs
	{
		ObsMessageReceiveStart = 0,

		// Observation Server receive Messages
		ObsMsg_AddPublisher				= ObsMessageReceiveStart+1,
		ObsMsg_RemovePublisher			= ObsMessageReceiveStart+2,
		ObsMsg_PublishDataPool			= ObsMessageReceiveStart+3,
		ObsMsg_PublishStream			= ObsMessageReceiveStart+4,
		ObsMsg_RemovePublication		= ObsMessageReceiveStart+5,		// 5
		ObsMsg_GetPublisherId			= ObsMessageReceiveStart+6,
		ObsMsg_GetPublicationId			= ObsMessageReceiveStart+7,
		ObsMsg_GetName					= ObsMessageReceiveStart+8,
		ObsMsg_SubscribeById			= ObsMessageReceiveStart+9,
		ObsMsg_SubscribeByName			= ObsMessageReceiveStart+10,	// 10
		ObsMsg_CancelSubscription		= ObsMessageReceiveStart+11,
		ObsMsg_RemoveSubscriber			= ObsMessageReceiveStart+12,
		ObsMsg_GetNumSubscribers		= ObsMessageReceiveStart+13,
		ObsMsg_EnumeratePublications	= ObsMessageReceiveStart+14,
		ObsMsg_EnumeratePublishers		= ObsMessageReceiveStart+15,	// 15
		ObsMsg_EnumerateSubscribers		= ObsMessageReceiveStart+16,
		ObsMsg_EnumerateSubscriptions	= ObsMessageReceiveStart+17,
		ObsMsg_GetDataPoolSize			= ObsMessageReceiveStart+18,
		ObsMsg_UpdateDataPool			= ObsMessageReceiveStart+19,
		ObsMsg_SendStream				= ObsMessageReceiveStart+20,	// 20
		ObsMsg_GetDataPool				= ObsMessageReceiveStart+21,
		ObsMsg_ReestablishPersistentTCP = ObsMessageReceiveStart+22,
		ObsMsg_AddSubscriber			= ObsMessageReceiveStart+23,
		// Add new receive messages here

		ObsMessageReceiveEnd = 99,

		ObsMessageSentStart = ObsMessageReceiveEnd+1,

		// Observation Server sent messages
		ObsMsg_StatusReply 					= ObsMessageSentStart+1,
		ObsMsg_DataPoolUpdated				= ObsMessageSentStart+2,
		ObsMsg_BroadcastDataPool			= ObsMessageSentStart+3,		
		ObsMsg_BroadcastStream				= ObsMessageSentStart+4,
		ObsMsg_PublishRequested				= ObsMessageSentStart+5,		// 105
		ObsMsg_SubscriptionCanceled			= ObsMessageSentStart+6,
		ObsMsg_PublicationSubscribedTo		= ObsMessageSentStart+7,
		ObsMsg_AddPublisherReply			= ObsMessageSentStart+8,
		ObsMsg_PublishReply					= ObsMessageSentStart+9,
		ObsMsg_SubscribeReply				= ObsMessageSentStart+10,		// 110
		ObsMsg_GetIdReply					= ObsMessageSentStart+11,
		ObsMsg_GetNameReply					= ObsMessageSentStart+12,
		ObsMsg_GetNumSubscribersReply		= ObsMessageSentStart+13,
		ObsMsg_GetDataPoolReply				= ObsMessageSentStart+14,
		ObsMsg_GetDataPoolSizeReply			= ObsMessageSentStart+15,		// 115
		ObsMsg_EnumeratePublishersReply		= ObsMessageSentStart+16,
		ObsMsg_EnumerateSubscribersReply	= ObsMessageSentStart+17,
		ObsMsg_EnumeratePublicationsReply	= ObsMessageSentStart+18,
		ObsMsg_EnumerateSubscriptionsReply	= ObsMessageSentStart+19,
		ObsMsg_AddSubscriberReply			= ObsMessageSentStart+20,		// 120
		// Add new sent messages here

		ObsMessageSentEnd
	};

};

#endif