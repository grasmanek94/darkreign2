////////////////////////////////////////////////////////////////////////////////
//
// Network Ping
//
// Copyright 1999-2000
// Matthew Versluys
//

//
// Ref: RFC 2068 HTTP 1.1
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "network_http.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Network
//
namespace Network
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace StatusCodes
  //
  namespace StatusCodes
  {
    const char * Continue                     = "100";
    const char * SwitchingProtocols           = "101";
    const char * OK                           = "200";
    const char * Created                      = "201";
    const char * Accepted                     = "202";
    const char * NonAuthoritativeInformation  = "203";
    const char * NoContent                    = "204";
    const char * ResetContent                 = "205";
    const char * PartialContent               = "206";
    const char * MultipleChoices              = "300";
    const char * MovedPermanently             = "301";
    const char * MovedTemporarily             = "302";
    const char * SeeOther                     = "303";
    const char * NotModified                  = "304";
    const char * UseProxy                     = "305";
    const char * BadRequest                   = "400";
    const char * Unauthorized                 = "401";
    const char * PaymentRequired              = "402";
    const char * Forbidden                    = "403";
    const char * NotFound                     = "404";
    const char * MethodNotAllowed             = "405";
    const char * NotAcceptable                = "406";
    const char * ProxyAuthenticationRequired  = "407";
    const char * RequestTimeout               = "408";
    const char * Conflict                     = "409";
    const char * Gone                         = "410";
    const char * LengthRequired               = "411";
    const char * PreconditionFailed           = "412";
    const char * RequestEntityTooLarge        = "413";
    const char * RequestURITooLarge           = "414";
    const char * UnsupportedMediaType         = "415";
    const char * InternalServerError          = "500";
    const char * NotImplemented               = "501";
    const char * BadGateway                   = "502";
    const char * ServiceUnavailable           = "503";
    const char * GatewayTimeout               = "504";
    const char * HTTPVersioNnotSupported      = "505";

  }


	const char ContentLength[] = "Content-Length:";
	const char ContentRange[] = "Content-Range:";
	const char Scheme[] = "http://";


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Http
  //

  // Open a connection to the given URL
  Bool Http::Open(const Url &url, U32 &offset, U32 &size)
  {
    url;
    offset;
    size;
    return (FALSE);
  }

  // Close a the connection
  Bool Http::Close()
  {
    return (FALSE);
  }

  // Read from the HTTP connection
  Bool Http::Read(U8 *buf, U32 size)
  {
    buf;
    size;
    return (FALSE);
  }

}

