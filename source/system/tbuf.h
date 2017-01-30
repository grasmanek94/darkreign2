///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Simple token reader for text buffers.
//
// 24-NOV-1997
//


#ifndef __TBUF_H
#define __TBUF_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
//  Definitions
//

// Size of the name buffer
#define TBUF_MAXIDENT    64

// max token len
#define TBUF_MAX_TOKEN   256

// space character
#define TBUF_CH_SPACE    32

// tab character
#define TBUF_CH_TAB      9

// end of line char
#define TBUF_CH_EOL      10

// carriage return char
#define TBUF_CH_CR       13

// token string
typedef char TokenStr[TBUF_MAX_TOKEN];

// function return values
enum TBufResult
{
  TR_OK, TR_PUN, TR_EOF
};

// error handler call-back
typedef void (FASTCALL ErrorCallBack)(const char *bufName, U32 xPos, U32 yPos, const char *errStr);

// String for the buffer name
typedef StrBuf<TBUF_MAXIDENT> TBufString;



///////////////////////////////////////////////////////////////////////////////
//
//  Class TBuf - Allows access to a text buffer as tokens
//

class TBuf
{
public:

  // char type values
  enum TBufCharType
  {
    NORMAL, PUNCTUATION, COMMENT
  };


private:

  // Are we currently setup
  Bool setup;

  // Peek state
  enum { PS_OFF, PS_PRE, PS_ON } peekState;

  // Buffer information
  const char *bufData;
  U32 bufSize;
  U32 bufPos;
  Bool commentOn;
  Bool constantOn;
  U8 constantChar;
  TBufString bufName;

  // Saved peeked return value
  TBufResult peekVal;
  
  // Parse position
  U32 xPos, yPos;
  U32 lastXPos;

  // Punctuation array
  TBufCharType pArray[U8_MAX+1];

  // Error handler
  ErrorCallBack *errHandler;


private:

  // Get the next char from the buffer
  Bool NextChar(U8 *c);

  // Steps back one char in the buffer
  void StepBack();

  // Returns the type of 'c'
  TBufCharType CharType(U8 c);

  // Fatal error handler
  static void FASTCALL DefaultError(const char *bufName, U32 x, U32 y, const char *errStr);


public:

  // Constructor and destructor
  TBuf();
  ~TBuf();

  // Setup with a buffer
  void Setup(const char *buffer, U32 size, const char *name, ErrorCallBack *err = NULL);

  // Finished parsing
  void Done();

  // Reads the next token from the file
  TBufResult NextToken();

  // Calls NextToken, but causes the next call to do nothing
  TBufResult PeekToken();

  // Calls NextToken, and generates error if punctuation or EOF
  void AcceptIdent();

  // Calls NextToken, and generates error if not punctuation
  void AcceptPunct();

  // Calls NextToken, and generates error if not 'expect'
  void Accept(const char *accept);

  // Generates error if lastToken is not 'expect'
  void Expect(const char *expect);

  // Start reading a constant, stopping on character 'c'
  void ReadConstant(U8 c);

  // Sets the type of each char in 'pStr' to 'type'
  void SetCharType(const char *pStr, TBufCharType type);

  // Resets the types of all chars to normal
  void ResetCharTypes();

  // Fatal: "Expecting %s but found '%s' with buffer name and line
  void ExpectError(const char *expected, const char *found);

  // Fatal: "Expecting %s but reached end of data"
  void EofError(const char *expected);

  // Fatal: triggers error with current parse position
  void CDECL TokenError(const char *fmt, ...);

  // Last token read
  TokenStr lastToken;

  // Previous token read
  TokenStr prevToken;

  // Peeked at token
  TokenStr peekToken;

  // Available for use in lists
  TBuf *listNext;

  // Returns the current text file line
  U32 CurrentLine()
  {
    ASSERT(setup);
    return (yPos);
  }

  // Returns the current position in the buffer
  U32 CurrentPos()
  {
    ASSERT(setup);
    return (bufPos);
  }

  // Returns string at current buffer pos
  const char *CurrentStr()
  {
    ASSERT(setup)
    ASSERT(bufData)
    return (bufData + bufPos);
  }
 
  // Returns the current buffer name
  const char *BufferName() 
  {
    ASSERT(setup);
    return (bufName.str);
  }
};


#endif
