#ifndef ROUTINGSERVERTYPES_H
#define ROUTINGSERVERTYPES_H

#include "common/won.h"

namespace WONMsg {

typedef unsigned short        ClientId;
typedef unsigned short        GroupId;
typedef unsigned short        ClientOrGroupId;
typedef WONCommon::RawBuffer  ClientName;
typedef std::string           GroupName;
typedef std::wstring          Password;
typedef std::list<ClientName> ClientNameList;

// used by ClientChangeEx and GetClientListReply
enum { OPTIONALFIELD_IP = 0x01, 
       OPTIONALFIELD_AUTHINFO = 0x02,
       OPTIONALFIELD_MUTEFLAG = 0x03, 
       OPTIONALFIELD_MODERATORFLAG = 0x04 };
enum { INCLUDE_IPS = 0x01, INCLUDE_AUTHINFO = 0x02 };
enum { NOT_INCLUDE_IPS = 0xFE, NOT_INCLUDE_AUTHINFO = 0xFD };

// used by ClientChangeEx and GetClientListReply
struct FieldInfo {
	unsigned char mType;
	unsigned char mSize;
	FieldInfo() : mType(0), mSize(0) {}
	FieldInfo(unsigned char theType, unsigned char theSize) : mType(theType), mSize(theSize) {}
};

// used by SendChat messages
enum ChatType { CHATTYPE_UNKNOWN       = 0, 
                CHATTYPE_ASCII         = 1, 
                CHATTYPE_ASCII_EMOTE   = 2, 
                CHATTYPE_UNICODE       = 3, 
                CHATTYPE_UNICODE_EMOTE = 4 };

// used by RoutingServerGenericFlagMessage and MMsgRoutingSimpleClientListReply
enum ContentType { CONTENTTYPE_UNKNOWN    =0,
		           CONTENTTYPE_CLIENTID   =1, 
				   CONTENTTYPE_CLIENTNAME =2, 
				   CONTENTTYPE_WONUSERID  =3 };

}; // namespace WONMsg

#endif // ROUTINGSERVERTYPES_H