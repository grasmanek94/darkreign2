#ifndef __WON_PING_H__
#define __WON_PING_H__


#include "Socket/WONWS.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Socket/IPSocket.h"
#include "Socket/RawIPSocket.h"


namespace WONAPI {


struct PingOnceResult
{
	IPSocket::Address addr;
	Error error;
	unsigned int pingTime;
	unsigned int* pingTimeArg;

	PingOnceResult(const IPSocket::Address& address, Error err, unsigned int time, unsigned int* timeArg)
		:	addr(address),
			error(err),
			pingTime(time),
			pingTimeArg(timeArg)
	{ }
};


struct PingResult
{
	IPSocket::Address addr;
	Error error;
	unsigned int pingTime;
	unsigned int averagePingTime;
	unsigned int* pingTimeArg;

	PingResult(const IPSocket::Address& address, Error err, unsigned int time, unsigned long average, unsigned int* timeArg)
		:	addr(address),
			error(err),
			pingTime(time),
			averagePingTime(average),
			pingTimeArg(timeArg)
	{ }
};


struct OrderServersResult
{
	Error error;
	IPSocket::Address* addrs;
	unsigned int numAddrs;
	unsigned int numGoodAddrs;
	unsigned int* numGoodAddrsArg;

	OrderServersResult(	Error err, IPSocket::Address* addresses, unsigned int numAddresses,
						unsigned int numGoodAddresses, unsigned int* numGoodAddressesArg)
		:	error(err),
			addrs(addresses),
			numAddrs(numAddresses),
			numGoodAddrs(numGoodAddresses),
			numGoodAddrsArg(numGoodAddressesArg)
	{ }
};


// IcmpPingOnce ----------------------------------------


Error IcmpPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, 
				  unsigned long timeout, bool async,
				  const CompletionContainer<const PingOnceResult&>& completion );

inline Error IcmpPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, 
						  unsigned long timeout = 1000, bool async = false)
{ return IcmpPingOnce(addr, pingTime, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
Error IcmpPingOnceEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, bool async, void (*f)(const PingOnceResult&, privsType), privsType t)
{ return IcmpPingOnce(addr, pingTime, timeout, async, new CompletionWithContents<const PingOnceResult&, privsType>(t, f, true)); }


// IcmpPing ----------------------------------------


Error IcmpPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
			   unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage,
			   bool async, const CompletionContainer<const PingResult&>& completion);

inline Error IcmpPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout = 2000,
			   unsigned int numPings = 4, unsigned long timeoutPerPing = 1000, bool setPingTimeArgToAverage = true,
			   bool async = false )
{ return IcmpPing(addr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage, async, DEFAULT_COMPLETION); }


template <class privsType>
Error IcmpPingEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage, bool async, void (*f)(const PingResult&, privsType), privsType t)
{ return IcmpPing(addr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage, async, new CompletionWithContents<const PingResult&, privsType>(t, f, true)); }


// TitanTCPPingOnce ----------------------------------------


Error TitanTCPPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout = 1000,
					   unsigned char lengthFieldSize = 4, bool includeConnectTime = true, bool async = false,
					   const CompletionContainer<const PingOnceResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
Error TitanTCPPingOnceEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, unsigned char lengthFieldSize, bool includeConnectTime, bool async,  void (*f)(const PingOnceResult&, privsType), privsType t)
{ return TitanTCPPingOnce(addr, pingTime, timeout, lengthFieldSize, includeConnectTime, async, new CompletionWithContents<const PingOnceResult&, privsType>(t, f, true)); }


// TitanTCPPing ----------------------------------------


Error TitanTCPPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout = 2000,
				unsigned char lengthFieldSize = 4, bool includeConnectTime = true, unsigned int numPings = 4,
				unsigned long timeoutPerPing = 1000, bool setPingTimeArgToAverage = true, bool async = false,
				const CompletionContainer<const PingResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
Error TitanTCPPingEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, unsigned char lengthFieldSize, bool includeConnectTime, unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage, bool async,  void (*f)(const PingResult&, privsType), privsType t)
{ return TitanTCPPing(addr, pingTime, timeout, lengthFieldSize, includeConnectTime, numPings, timeoutPerPing, setPingTimeArgToAverage, async, new CompletionWithContents<const PingResult&, privsType>(t, f, true)); }


// TitanUDPPingOnce ----------------------------------------


Error TitanUDPPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout = 1000,
					   bool async = false, const CompletionContainer<const PingOnceResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
Error TitanUDPPingOnceEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, bool async, void (*f)(const PingOnceResult&, privsType), privsType t)
{ return TitanUDPPingOnce(addr, pingTime, timeout, async, new CompletionWithContents<const PingOnceResult&, privsType>(t, f, true)); }


// TitanUDPPing ----------------------------------------


Error TitanUDPPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout = 2000,
				   unsigned int numPings = 4, unsigned long timeoutPerPing = 1000,
				   bool setPingTimeArgToAverage = true, bool async = false,
				   const CompletionContainer<const PingResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
Error TitanUDPPingEx(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage, bool async, void (*f)(const PingResult&, privsType), privsType t)
{ return TitanUDPPing(addr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage, async, new CompletionWithContents<const PingResult&, privsType>(t, f, true)); }


// OrderServersByIcmpPing ----------------------------------------

Error OrderServersByIcmpPing(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray,
							 unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
							 unsigned int numPingsPerServer, unsigned long timeoutPerServer,
							 unsigned long timeoutPerPing, bool useAveragePing, bool async,
							 const CompletionContainer<const OrderServersResult&>& completion );

inline Error OrderServersByIcmpPing(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray = 0,
							 unsigned int* numGoodAddrs = 0, unsigned int maxSimultaniousPings = 1,
							 unsigned int numPingsPerServer = 4, unsigned long timeoutPerServer = 2000,
							 unsigned long timeoutPerPing = 1000, bool useAveragePing = true, bool async = false)
{ return OrderServersByIcmpPing(addresses, numAddresses, pingArray, numGoodAddrs, maxSimultaniousPings, numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing, async, DEFAULT_COMPLETION); }

template <class privsType>
Error OrderServersByIcmpPingEx(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray, unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings, unsigned int numPingsPerServer, unsigned long timeoutPerServer, unsigned long timeoutPerPing, bool useAveragePing, bool async, void (*f)(const OrderServersResult&, privsType), privsType t)
{ return OrderServersByIcmpPing(addresses, numAddresses, pingArray, numGoodAddrs, maxSimultaniousPings, numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing, async, new CompletionWithContents<const OrderServersResult&, privsType>(t, f, true)); }



// OrderServersByTitanTCPPing ----------------------------------------


Error OrderServersByTitanTCPPing(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray = 0,
								 unsigned char lengthFieldSize = 4, bool includeConnectTime = true, 
								 unsigned int* numGoodAddrs = 0, unsigned int maxSimultaniousPings = 1,
								 unsigned int numPingsPerServer = 4, unsigned long timeoutPerServer = 2000,
								 unsigned long timeoutPerPing = 1000, bool useAveragePing = true, bool async = false,
								 const CompletionContainer<const OrderServersResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
Error OrderServersByTitanTCPPingEx(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray, unsigned char lengthFieldSize, bool includeConnectTime,  unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings, unsigned int numPingsPerServer, unsigned long timeoutPerServer, unsigned long timeoutPerPing, bool useAveragePing, bool async, void (*f)(const OrderServersResult&, privsType), privsType t)
{ return OrderServersByTitanTCPPing(addresses, numAddresses, pingArray, lengthFieldSize, includeConnectTime, numGoodAddrs, maxSimultaniousPings, numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing, async, new CompletionWithContents<const OrderServersResult&, privsType>(t, f, true)); }




// OrderServersByTitanUDPPing ----------------------------------------


Error OrderServersByTitanUDPPing(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray = 0,
								 unsigned int* numGoodAddrs = 0, unsigned int maxSimultaniousPings = 1,
								 unsigned int numPingsPerServer = 4, unsigned long timeoutPerServer = 2000,
								 unsigned long timeoutPerPing = 1000, bool useAveragePing = true, bool async = false,
								 const CompletionContainer<const OrderServersResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
Error OrderServersByTitanUDPPingEx(IPSocket::Address* addresses, unsigned int numAddresses, unsigned int* pingArray, unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings, unsigned int numPingsPerServer, unsigned long timeoutPerServer, unsigned long timeoutPerPing, bool useAveragePing, bool async, void (*f)(const OrderServersResult&, privsType), privsType t)
{ return OrderServersByTitanUDPPing(addresses, numAddresses, pingArray, numGoodAddrs, maxSimultaniousPings, numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing, async, new CompletionWithContents<const OrderServersResult&, privsType>(t, f, true)); }


};


#endif
