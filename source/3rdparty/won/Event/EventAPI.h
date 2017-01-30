#ifndef __WON_EVENT_API_H__
#define __WON_EVENT_API_H__


#include "common/won.h"
#include <time.h>
#include <string>
#include "common/WONString.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "msg/Event/SMsgEventRecordEvent.h"
#include "Authentication/Identity.h"
#include "Socket/IPSocket.h"


namespace WONAPI {


	// Not to be confused with windows events, or WONCommon::Event
class WONEvent
{
public:
	WONMsg::SMsgEventRecordEvent msg;
	

	WONEvent()
	{
	}


	WONEvent(unsigned short activityType)
	{
		SetActivityType(activityType);
	}


	void SetActivityType(unsigned short activityType)
	{
		msg.SetActivityType(activityType);
	}


	void SetActivityDateTime(time_t activityDateTime = time(0))
	{
		msg.SetHasDateTime(true);
		msg.SetActivityDateTime(activityDateTime);
	}


	void SetServer(unsigned short serverType, const WONCommon::WONString& logicalName, const std::string& serverAddr)
	{
		msg.SetHasRelatedServer(true);
		msg.SetServerType(serverType);
		msg.SetServerLogicalName(logicalName);
		msg.SetServerNetAddress(serverAddr);
	}


	void SetClient(const WONCommon::WONString& clientName, const std::string& clientAddr)
	{
		msg.SetHasRelatedClient(true);
		msg.SetClientName(clientName);
		msg.SetClientNetAddress(clientAddr);
	}


	void SetUser(unsigned short authMethod, unsigned long userId, const WONCommon::WONString& userName)
	{
		msg.SetHasRelatedUser(true);
		msg.SetUserAuthenticationMethod(authMethod);
		msg.SetUserId(userId);
		msg.SetUserName(userName);
	}


	void AddDetail(unsigned short detailType, const WONCommon::WONString& str)
	{
		msg.AddDetail(detailType, str);
	}


	void AddDetail(unsigned short detailType, double d)
	{
		msg.AddDetail(detailType, d);
	}


	void AddAttachment(const WONCommon::WONString& desc, BYTE contentType, void* body, unsigned short size)
	{
		msg.AddAttachment(desc, contentType, WONDatabase::DBBlob((unsigned char*)body, size));
	}
};



// Send events to event server
Error ReportEvents(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
				   const WONEvent* evts, unsigned int numEvts, bool reliable,
				   bool useUDP, long timeout, long perUDPtimeout, bool async, const CompletionContainer<Error>& completion);

inline Error ReportEvents(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
						  const WONEvent* evts, unsigned int numEvts = 1, bool reliable = true,
						  bool useUDP = true, long timeout = -1, long perUDPtimeout = 2000, bool async = false)
	{ return ReportEvents(ident, eventSrvrs, numSrvrs, evts, numEvts, reliable, useUDP, timeout, perUDPtimeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error ReportEventsEx(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
							const WONEvent* evts, unsigned int numEvts, bool reliable, bool useUDP,
							long timeout, long perUDPtimeout, bool async, void (*f)(Error, privsType), privsType privs)
	{ return ReportEvents(ident, eventSrvrs, numSrvrs, evts, numEvts, reliable, useUDP, timeout, perUDPtimeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


inline Error ReportEvents(Identity* ident, const IPSocket::Address& eventSrvr, const WONEvent* evts,
						  unsigned int numEvts = 1, bool reliable = true, bool useUDP = true,
						  long timeout = -1, long perUDPtimeout = 2000, bool async = false,
						  const CompletionContainer<Error>& completion = DEFAULT_COMPLETION)
	{ return ReportEvents(ident, &eventSrvr, 1, evts, numEvts, reliable, useUDP, timeout, perUDPtimeout, async, completion); }


template <class privsType>
inline Error ReportEventsEx(Identity* ident, const IPSocket::Address& eventSrvr, const WONEvent* evts,
							unsigned int numEvts, bool reliable, bool useUDP,
							long timeout, long perUDPtimeout, bool async, void (*f)(Error, privsType), privsType privs)
	{ return ReportEvents(ident, &eventSrvr, 1, evts, numEvts, reliable, useUDP, timeout, perUDPtimeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


inline Error ReportEvent(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
						 const WONEvent& evt, bool reliable = true, bool useUDP = true,
						 long timeout = -1, long perUDPtimeout = 2000, bool async = false,
						 const CompletionContainer<Error>& completion = DEFAULT_COMPLETION)
	{ return ReportEvents(ident, eventSrvrs, numSrvrs, &evt, 1, reliable, useUDP, timeout, perUDPtimeout, async,completion); }


template <class privsType>
inline Error ReportEventEx(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
						   const WONEvent& evt, bool reliable, bool useUDP, 
						   long timeout, long perUDPtimeout, bool async, void (*f)(Error, privsType), privsType privs)
	{ return ReportEvents(ident, eventSrvrs, numSrvrs, &evt, 1, reliable, useUDP, timeout, perUDPtimeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


inline Error ReportEvent(Identity* ident, const IPSocket::Address& eventSrvr, const WONEvent& evt,
						 bool reliable = true, bool useUDP = true, long timeout = -1, long perUDPtimeout = 2000,
						 bool async = false, const CompletionContainer<Error>& completion = DEFAULT_COMPLETION)
	{ return ReportEvents(ident, &eventSrvr, 1, &evt, 1, reliable, useUDP, timeout, perUDPtimeout, async, completion); }


template <class privsType>
inline Error ReportEventEx(Identity* ident, const IPSocket::Address& eventSrvr, const WONEvent& evt,
						   bool reliable, bool useUDP, long timeout, long perUDPtimeout, bool async,
						   void (*f)(Error, privsType), privsType privs)
	{ return ReportEvents(ident, &eventSrvr, 1, &evt, 1, reliable, useUDP, timeout, perUDPtimeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


};


#endif
