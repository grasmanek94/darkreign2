#ifndef __WON_FIREWALLDETECT_API_H__
#define __WON_FIREWALLDETECT_API_H__


#include "Socket/IPSocket.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"


namespace WONAPI {


struct DetectFirewallResult
{
	Error error;
	bool* behindFirewall;

	DetectFirewallResult(Error err, bool* isBehindFirewall)
		:	error(err), behindFirewall(isBehindFirewall)
	{ }
};


// One addr

Error DetectFirewall(const IPSocket::Address& firewallDetectionServerAddr, bool* behindFirewall,
					 unsigned short useListenPort, long timeout, bool async,
					 const CompletionContainer<const DetectFirewallResult&>& completion );


inline Error DetectFirewall(const IPSocket::Address& firewallDetectionServerAddr, bool* behindFirewall,
							unsigned short useListenPort = 0, long timeout = -1, bool async = false)
{ return DetectFirewall(firewallDetectionServerAddr, behindFirewall, useListenPort, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error DetectFirewallEx(const IPSocket::Address& firewallDetectionServerAddr, bool* behindFirewall,
							unsigned short useListenPort, long timeout, bool async,
							void (*f)(const DetectFirewallResult&, privsType), privsType privs)
{ return DetectFirewall(firewallDetectionServerAddr, behindFirewall, useListenPort, timeout, async, new CompletionWithContents<const DetectFirewallResult&, privsType>(f, privs, true)); }



// With fail-over list of addresses

Error DetectFirewall(const IPSocket::Address* firewallDetectionServerAddrs, unsigned short numAddrs,
					 bool* behindFirewall, unsigned short useListenPort = 0, long timeout = -1,
					 bool async = false, const CompletionContainer<const DetectFirewallResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error DetectFirewallEx(const IPSocket::Address* firewallDetectionServerAddrs, unsigned short numAddrs,
							bool* behindFirewall, unsigned short useListenPort, long timeout,
							bool async, void (*f)(const DetectFirewallResult&, privsType), privsType privs)
{ return DetectFirewall(firewallDetectionServerAddrs, numAddrs, behindFirewall, useListenPort, timeout, async, new CompletionWithContents<const DetectFirewallResult&, privsType>(f, privs, true)); }


};


#endif
