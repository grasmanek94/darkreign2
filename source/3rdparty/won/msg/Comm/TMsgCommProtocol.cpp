#include <messages/BadMsgException.h>

#include <TMsgCommProtocol.h>

namespace {
      using WONMsg::TMessage;
      using WONMsg::TMsgCommProtocol;
      using WONMsg::TMsgCommProtocolReject;
};

// default ctor
TMsgCommProtocol::TMsgCommProtocol()
      : TMessage(),
        mProtocolType( TProtNone ),
        mNestedMessage()
{
    this->SetServiceType( WONMsg::CommonService );
    this->SetMessageType( WONMsg::CommProtocol );
}

// TMessage ctor
TMsgCommProtocol::TMsgCommProtocol( const TMessage& theMessage )
      : TMessage( theMessage )
{
    Unpack();
}

// copy ctor
TMsgCommProtocol::TMsgCommProtocol( const TMsgCommProtocol& theMsg )
      : TMessage( theMsg ),
        mProtocolType( theMsg.mProtocolType ),
        mNestedMessage( theMsg.mNestedMessage ) // packed TMessage
{
}

// Destructor
TMsgCommProtocol::~TMsgCommProtocol() {}

// Assignment operator
TMsgCommProtocol&
TMsgCommProtocol::operator=(const TMsgCommProtocol& theMsgR)
{
    TMessage::operator=(theMsgR);
    mProtocolType   = theMsgR.mProtocolType;
    mNestedMessage  = theMsgR.mNestedMessage;
    return *this;
}

void *TMsgCommProtocol::Pack()
{
    WTRACE("TMsgCommProtocol::Pack");

    TMessage::Pack();

    WDBG_LL("TMsgCommProtocol::Pack Appending message data");
    AppendByte( mProtocolType );
    AppendTMessage( mNestedMessage );

    return GetDataPtr();
}

void TMsgCommProtocol::Unpack()
{
    WTRACE("TMsgCommProtocol::Unpack");
    TMessage::Unpack();

    if ((GetServiceType() != WONMsg::CommonService) ||
        (GetMessageType() != WONMsg::CommProtocol))
    {
       WDBG_AH("TMsgCommProtocol::Unpack Not a CommProtocol message!");
       throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
                           "Not a CommProtocol message.");
    }

    WDBG_LL("TMsgCommProtocol::Unpack Reading message data");
    mProtocolType  = MsgProtocolType( ReadByte() );
    mNestedMessage = ReadTMessage();
}

// default ctor
TMsgCommProtocolReject::TMsgCommProtocolReject()
      : TMessage(),
        mProtocolType( TProtNone )
{
    this->SetServiceType( WONMsg::CommonService );
    this->SetMessageType( WONMsg::CommProtocolReject );
}

// TMessage ctor
TMsgCommProtocolReject::TMsgCommProtocolReject( const TMessage& theMessage )
      : TMessage( theMessage )
{
    Unpack();
}

// copy ctor
TMsgCommProtocolReject::TMsgCommProtocolReject( const TMsgCommProtocolReject& theMsg )
      : TMessage( theMsg ),
        mProtocolType( theMsg.mProtocolType )
{
}

// Destructor
TMsgCommProtocolReject::~TMsgCommProtocolReject() {}

// Assignment operator
TMsgCommProtocolReject&
TMsgCommProtocolReject::operator=(const TMsgCommProtocolReject& theMsgR)
{
    TMessage::operator=(theMsgR);
    mProtocolType   = theMsgR.mProtocolType;
    return *this;
}

void *TMsgCommProtocolReject::Pack()
{
    WTRACE("TMsgCommProtocolReject::Pack");

    TMessage::Pack();

    WDBG_LL("TMsgCommProtocolReject::Pack Appending message data");
    AppendByte( mProtocolType );

    return GetDataPtr();
}

void TMsgCommProtocolReject::Unpack()
{
    WTRACE("TMsgCommProtocolReject::Unpack");
    TMessage::Unpack();

    if ((GetServiceType() != WONMsg::CommonService) ||
        (GetMessageType() != WONMsg::CommProtocolReject))
    {
       WDBG_AH("TMsgCommProtocol::Unpack Not a CommProtocolReject message!");
       throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
                           "Not a CommProtocolReject message.");
    }

    WDBG_LL("TMsgCommProtocolReject::Unpack Reading message data");
    mProtocolType  = MsgProtocolType( ReadByte() );
}