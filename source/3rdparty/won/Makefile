OBJS = Errors.o \
      Ping.o \
      WONHTTP.o \
      SDKCommon/Completion.o \
      SDKCommon/MemStream.o \
      SDKCommon/Stream.o \
      Socket/IPSocket.o \
      Socket/RawIPSocket.o \
      Socket/TCPSocket.o \
      Socket/IPXSocket.o \
      Socket/SPXSocket.o \
      Socket/TMsgSocket.o \
      Socket/WSSocket.o \
      Socket/PXSocket.o \
      Socket/WONSocket.o \
      Socket/UDPSocket.o \
      auth/Auth1Certificate.o \
      auth/Auth1PrivateKeyBlock.o \
      auth/Auth1PublicKeyBlock.o \
      auth/AuthFamilyBuffer.o \
      auth/AuthPublicKeyBlockBase.o \
      common/WONString.o \
      common/CRC16.o \
      common/DataObject.o \
      common/EventLog.o \
      common/Utils.o \
      common/WONException.o \
      crypt/BFSymmetricKey.o \
      crypt/CryptKeyBase.o \
      crypt/EGPrivateKey.o \
      crypt/EGPublicKey.o \
      crypt/MD5Hash.o \
      crypt/cryptoFiles/algebra.o \
      crypt/cryptoFiles/asn.o \
      crypt/cryptoFiles/bfinit.o \
      crypt/cryptoFiles/blowfish.o \
      crypt/cryptoFiles/cbc.o \
      crypt/cryptoFiles/cryptlib.o \
      crypt/cryptoFiles/elgamal.o \
      crypt/cryptoFiles/eprecomp.o \
      crypt/cryptoFiles/filters.o \
      crypt/cryptoFiles/integer.o \
      crypt/cryptoFiles/iterhash.o \
      crypt/cryptoFiles/md5.o \
      crypt/cryptoFiles/misc.o \
      crypt/cryptoFiles/modes.o \
      crypt/cryptoFiles/modexppc.o \
      crypt/cryptoFiles/nbtheory.o \
      crypt/cryptoFiles/nr.o \
      crypt/cryptoFiles/pch.o \
      crypt/cryptoFiles/pubkey.o \
      crypt/cryptoFiles/queue.o \
      crypt/cryptoFiles/randpool.o \
      msg/BadMsgException.o \
      msg/TMessage.o \
      msg/Contest/MMsgContestDBCall.o \
      msg/Auth/AuthMsgs.o \
      msg/Comm/CommMsgs.o \
      msg/Routing/RoutingMsgs.o \
      msg/Firewall/FirewallMsgs.o \
      msg/Fact/FactMsgs.o \
      msg/Dir/DirMsgs.o \
      msg/Dir/DirMsgsG2.o \
      msg/Obs/ObsMsgs.o \
      msg/Profile/SMsgProfileMsgs.o \
      Authentication/AuthSocket.o \
      Authentication/ClientCDKey.o \
      Authentication/Identity.o \
      Authentication/Md52.o \
      Contest/ContestAPI.o \
      Directory/DirectoryAPI.o \
      Directory/Legacy.o \
      Factory/FactoryAPI.o \
      Firewall/FirewallAPI.o \
      Observation/ObservationAPI.o \
      Observation/SocketMgr.o \
      Routing/RoutingAPI.o \
      Routing/RoutingAPITypes.o \
      Routing/CRoutingAPI.o \
      Profile/ProfileAPI.o \
      Event/EventAPI.o \
      crypt/Randomizer.o

SRCS = Errors.cpp \
      Ping.cpp \
      WONHTTP.cpp \
      SDKCommon/Completion.cpp \
      SDKCommon/MemStream.cpp \
      SDKCommon/Stream.cpp \
      Socket/IPSocket.cpp \
      Socket/RawIPSocket.cpp \
      Socket/TCPSocket.cpp \
      Socket/IPXSocket.cpp \
      Socket/SPXSocket.cpp \
      Socket/TMsgSocket.cpp \
      Socket/WSSocket.cpp \
      Socket/PXSocket.cpp \
      Socket/WONSocket.cpp \
      Socket/UDPSocket.cpp \
      auth/Auth1Certificate.cpp \
      auth/Auth1PrivateKeyBlock.cpp \
      auth/Auth1PublicKeyBlock.cpp \
      auth/AuthFamilyBuffer.cpp \
      auth/AuthPublicKeyBlockBase.cpp \
      common/WONString.cpp \
      common/CRC16.cpp \
      common/DataObject.cpp \
      common/EventLog.cpp \
      common/Utils.cpp \
      common/WONException.cpp \
      crypt/BFSymmetricKey.cpp \
      crypt/CryptKeyBase.cpp \
      crypt/EGPrivateKey.cpp \
      crypt/EGPublicKey.cpp \
      crypt/MD5Hash.cpp \
      crypt/cryptoFiles/algebra.cpp \
      crypt/cryptoFiles/asn.cpp \
      crypt/cryptoFiles/bfinit.cpp \
      crypt/cryptoFiles/blowfish.cpp \
      crypt/cryptoFiles/cbc.cpp \
      crypt/cryptoFiles/cryptlib.cpp \
      crypt/cryptoFiles/elgamal.cpp \
      crypt/cryptoFiles/eprecomp.cpp \
      crypt/cryptoFiles/filters.cpp \
      crypt/cryptoFiles/integer.cpp \
      crypt/cryptoFiles/iterhash.cpp \
      crypt/cryptoFiles/md5.cpp \
      crypt/cryptoFiles/misc.cpp \
      crypt/cryptoFiles/modes.cpp \
      crypt/cryptoFiles/modexppc.cpp \
      crypt/cryptoFiles/nbtheory.cpp \
      crypt/cryptoFiles/nr.cpp \
      crypt/cryptoFiles/pch.cpp \
      crypt/cryptoFiles/pubkey.cpp \
      crypt/cryptoFiles/queue.cpp \
      crypt/cryptoFiles/randpool.cpp \
      msg/BadMsgException.cpp \
      msg/TMessage.cpp \
      msg/Contest/MMsgContestDBCall.cpp \
      msg/Auth/AuthMsgs.cpp \
      msg/Comm/CommMsgs.cpp \
      msg/Routing/RoutingMsgs.cpp \
      msg/Firewall/FirewallMsgs.cpp \
      msg/Fact/FactMsgs.cpp \
      msg/Dir/DirMsgs.cpp \
      msg/Dir/DirMsgsG2.cpp \
      msg/Obs/ObsMsgs.cpp \
      msg/Profile/SMsgProfileMsgs.cpp \
      Authentication/AuthSocket.cpp \
      Authentication/ClientCDKey.cpp \
      Authentication/Identity.cpp \
      Authentication/Md52.cpp \
      Contest/ContestAPI.cpp \
      Directory/DirectoryAPI.cpp \
      Directory/Legacy.cpp \
      Factory/FactoryAPI.cpp \
      Firewall/FirewallAPI.cpp \
      Observation/ObservationAPI.cpp \
      Observation/SocketMgr.cpp \
      Routing/RoutingAPI.cpp \
      Routing/RoutingAPITypes.cpp \
      Routing/CRoutingAPI.cpp \
      Profile/ProfileAPI.cpp \
      Event/EventAPI.cpp \
      crypt/Randomizer.cpp

CFLAGS= -g -D_LINUX -D_NO_TOPLEVEL_CATCH -I.
CPPFLAGS = $(CFLAGS)

all: libTitan.a
	
clean:
	find . -name '*.o' -type f -exec rm {} \;
	rm libTitan.a

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

libTitan.a: $(OBJS)
	$(AR) $(ARFLAGS)s libTitan.a $(OBJS)

wonlogin: wonlogin.o libTitan.a
	g++ -o wonlogin -lpthread wonlogin.o libTitan.a

