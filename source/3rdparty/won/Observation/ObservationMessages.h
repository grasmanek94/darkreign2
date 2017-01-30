#ifndef _ObsMessages_H
#define _ObsMessages_H

///////////////////////////////////////////////////////////////////////////////////
//
// Obs server Messages
//
// Contains classes for messages that can be sent to and are received from the 
// Obs server.
//
///////////////////////////////////////////////////////////////////////////////////

#include <msg/BadMsgException.h>
#include <msg/TMessage.h>
#include <msg/MServiceTypes.h>
#include <msg/ServerStatus.h>
#include <msg/Comm/MMsgTypesComm.h>
#include <msg/Obs/MMsgTypesObs.h>
#include <msg/Obs/ObsServerListElements.h>
#include <msg/Obs/MMsgObsAddPublisher.h>
#include <msg/Obs/MMsgObsBroadcastDataPool.h>
#include <msg/Obs/MMsgObsBroadcastStream.h>
#include <msg/Obs/MMsgObsCancelSubscription.h>
#include <msg/Obs/MMsgObsDataPoolUpdated.h>
#include <msg/Obs/MMsgObsEnumPublications.h>
#include <msg/Obs/MMsgObsEnumPublishers.h>
#include <msg/Obs/MMsgObsEnumSubscribers.h>
#include <msg/Obs/MMsgObsEnumSubscriptions.h>
#include <msg/Obs/MMsgObsGetDataPool.h>
#include <msg/Obs/MMsgObsGetDataPoolSize.h>
#include <msg/Obs/MMsgObsGetIdReply.h>
#include <msg/Obs/MMsgObsGetName.h>
#include <msg/Obs/MMsgObsGetNumSubscribers.h>
#include <msg/Obs/MMsgObsGetPublicationId.h>
#include <msg/Obs/MMsgObsGetPublisherId.h>
#include <msg/Obs/MMsgObsPublicationSubTo.h>
#include <msg/Obs/MMsgObsPublishDataPool.h>
#include <msg/Obs/MMsgObsPublishReply.h>
#include <msg/Obs/MMsgObsPublishRequested.h>
#include <msg/Obs/MMsgObsPublishStream.h>
#include <msg/Obs/MMsgObsRemovePublication.h>
#include <msg/Obs/MMsgObsRemovePublisher.h>
#include <msg/Obs/MMsgObsRemoveSubscriber.h>
#include <msg/Obs/MMsgObsSendStream.h>
#include <msg/Obs/MMsgObsStatusReply.h>
#include <msg/Obs/MMsgObsSubscribeById.h>
#include <msg/Obs/MMsgObsSubscribeByName.h>
#include <msg/Obs/MMsgObsSubscribeReply.h>
#include <msg/Obs/MMsgObsSubscriptionCanceled.h>
#include <msg/Obs/MMsgObsUpdateDataPool.h>
#include <msg/Obs/MMsgObsReestabPersistTCP.h>
#include <msg/Obs/MMsgObsAddSubscriber.h>

#endif