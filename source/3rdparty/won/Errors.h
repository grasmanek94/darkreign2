#ifndef __WON_API_ERRORS_H__
#define __WON_API_ERRORS_H__


namespace WONAPI
{


typedef long Error;


const Error Error_Success					= 0;		// universal success constant

// Values in the range of a short are reserved for Titan server status values

const Error Error_OutOfMemory				= 100001;
const Error Error_GeneralFailure			= 100002;
const Error Error_InvalidState				= 100003;
const Error Error_NetFailed					= 100004;
const Error Error_NetResourceUnavailable	= 100005;
const Error Error_BadAddress				= 100006;
const Error Error_AddressInUse				= 100007;
const Error Error_HostUnreachable			= 100008;
const Error Error_Timeout					= 100009;
const Error Error_Pending					= 100010;
const Error Error_ConnectionClosed			= 100011;
const Error Error_InvalidMessage			= 100012;
const Error Error_Winsock2Required			= 100013;
const Error Error_InvalidParams				= 100014;
const Error Error_Unimplemented				= 100015;
const Error Error_Aborted					= 100016;
const Error Error_UnableToOpenFile			= 100017;
const Error Error_UnableToWriteToFile		= 100018;

// Error codes generated only by authentication
const Error Error_InvalidVerifierKey		= 110001;
const Error Error_GetPubKeysFailed			= 110002;
const Error Error_InvalidPubKeys			= 110003;
const Error Error_FailedToGenerateKey		= 110004;	// Pretty darn rare
const Error Error_InvalidAuthLoginReply		= 110005;
const Error Error_InvalidAuthLogin			= 110006;
const Error Error_BadNewPassword			= 110007;
const Error Error_ExpiredKey				= -1500;
const Error Error_VerifyFailed				= -1501;
const Error Error_LockedOut					= -1502;
const Error Error_KeyInUse					= -1503;
const Error Error_CRCFailed					= -1504;
const Error Error_UserExists				= -1505;
const Error Error_UserNotFound				= -1506;
const Error Error_BadPassword				= -1507;
const Error Error_DirtyWord					= -1508;
const Error Error_BadCommunity				= -1509;
const Error Error_InvalidCDKey				= -1510;
const Error Error_NotInCommunity			= -1511;

// Error codes generated only by peer-to-peer authentication
const Error Error_InvalidSessionKey 		= 120001;
const Error Error_InvalidCertificate		= 120002;
const Error Error_InvalidRemoteCertificate 	= 120003;
const Error Error_InvalidPrivateKey			= 120004;
const Error Error_EncryptFailed				= 120005;
const Error Error_InvalidSecret				= 120006;
//const Error Error_InvalidPubKeys			= 110003;	//already defined
//const Error Error_FailedToGenerateKey		= 110004;	// Pretty darn rare
const Error Error_Failure					= -1;
const Error Error_InvalidParameters			= -4;
const Error Error_NotSupported				= -12;


// Some APIs return server status codes which haven't migrated into this file yet.
// See: API/msg/ServerStatus.h
};

#endif
