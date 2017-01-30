

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFirewall.h"
#include "SMsgFirewallDetect.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFirewallDetect;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgFirewallDetect::SMsgFirewallDetect(void) : mListenPort(0),mWaitForConnect(false), mMaxConnectTime(0)
{
	SetServiceType(WONMsg::SmallFirewallDetector);
	SetMessageType(WONMsg::FirewallDetectMsg);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgFirewallDetect::~SMsgFirewallDetect(void)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgFirewallDetect::SMsgFirewallDetect(const SMsgFirewallDetect& theMsgR) : SmallMessage(theMsgR),
	mListenPort(theMsgR.mListenPort), mWaitForConnect(theMsgR.mWaitForConnect), mMaxConnectTime(theMsgR.mMaxConnectTime), mSendReplyToAcceptor(theMsgR.mSendReplyToAcceptor)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgFirewallDetect::SMsgFirewallDetect(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mListenPort(0), mWaitForConnect(false), mMaxConnectTime(0), mSendReplyToAcceptor(true)
{
	Unpack();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgFirewallDetect& SMsgFirewallDetect::operator =(const SMsgFirewallDetect& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mListenPort = theMsgR.mListenPort;
	mWaitForConnect = theMsgR.mWaitForConnect;
	mMaxConnectTime = theMsgR.mMaxConnectTime;
	mSendReplyToAcceptor = theMsgR.mSendReplyToAcceptor;

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void* SMsgFirewallDetect::Pack(void){

	WTRACE("SMsgFirewallDetect::Pack");

	SetServiceType(WONMsg::SmallFirewallDetector);
	SetMessageType(WONMsg::FirewallDetectMsg);
	SmallMessage::Pack();

	WDBG_LL("SMsgFirewallDetect::Pack Appending message data");
	AppendShort(mListenPort);
	AppendByte(mWaitForConnect);
	AppendLong(mMaxConnectTime);
	AppendByte(mSendReplyToAcceptor);

	return GetDataPtr();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SMsgFirewallDetect::Unpack(void){

	WTRACE("SMsgFirewallDetect::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFirewallDetector) ||
	    (GetMessageType() != WONMsg::FirewallDetectMsg))
	{
		WDBG_AH("SMsgFirewallDetect::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFirewallDetect::Unpack Reading message data");
	mListenPort = ReadShort();
	mWaitForConnect = ReadByte() ? true : false;
	mMaxConnectTime = ReadLong();
	if(BytesLeftToRead()>0)
		mSendReplyToAcceptor = ReadByte() ? true : false;
	else
		mSendReplyToAcceptor = true;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
