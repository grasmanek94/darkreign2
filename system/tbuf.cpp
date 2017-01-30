///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Simple token reader for text buffers.
//
// 24-NOV-1997
//

#include "tbuf.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Class TBuf - Allows access to a text buffer as tokens
//


//
// TBuf::TBuf()
//
// Constructor
//
TBuf::TBuf()
{  
  // Reset char type settings
  ResetCharTypes();

  // Clear setup flag
  setup = FALSE;
}


// 
// TBuf::Close()
//
// Destructor
//
TBuf::~TBuf()
{
  // Shutdown if still open
  Done();
}


// 
// TBuf::Setup
//
// Setup with a buffer
//
void TBuf::Setup(const char *buffer, U32 size, const char *name, ErrorCallBack *err)
{
  ASSERT(buffer);
  ASSERT(name);
  
  // Ensure we're shutdown
  Done();

  // Setup buffer information
  bufSize = size;
  bufData = buffer;
  bufName = name;
  bufPos = xPos = 0;
  yPos = 1;
  commentOn = FALSE;
  constantOn = FALSE;
  setup = TRUE;
  peekState = PS_OFF;

  // Clear token buffers
  *lastToken = '\0';
  *prevToken = '\0'; 
  *peekToken = '\0'; 

  // Set error handler
  errHandler = (err) ? err : DefaultError;
}


// 
// TBuf::Done
//
// Finished parsing
//
void TBuf::Done()
{
  // Are we active
  if (setup)
  {
    // This used to do something ok!
    setup = FALSE;
  }
}


//
// TBuf::NextToken
//
// Reads the next token into 'dest'. Skips any preceeding spaces, 
// stopping when either a space, a punctuation char, or end of data
// occurs.  Generates an error if token exceeds max token length.
//
// Returns :
// 
//  TR_OK   - if token was read with no problems
//  TR_PUN  - if punctuation char was found
//  TR_EOF  - if end of data reached
//
TBufResult TBuf::NextToken()
{
  ASSERT(setup);

  U8 c;
  U32 tokenLen = 0;
  char *sPtr;

  // Have we peeked at the next token
  if (peekState == PS_ON)
  {
    peekState = PS_OFF;
    Utils::Strcpy(prevToken, lastToken);
    Utils::Strcpy(lastToken, peekToken);
    return (peekVal);
  }

  // Setup peek operation
  if (peekState == PS_PRE)
  {
    // Store next token in peek buffer
    sPtr = peekToken;

    // We are now in peek mode
    peekState = PS_ON;
  }
  else
  { 
    // Save previous token
    Utils::Strcpy(prevToken, lastToken);
  
    // Store next token in last token buffer
    sPtr = lastToken;
  }

  // Clear token buffer
  *sPtr = '\0';

  // Read next token
  for (;;)
  {
    // Have we reached the end of the data
    if (!NextChar(&c))
    {
      // Return token if we have one
      return (tokenLen ? TR_OK : TR_EOF);
    }

    // Finish reading constant
    if (constantOn && c == constantChar)
    { 
      constantOn = FALSE;
    }
   
    // Ignore these checks if reading a constant
    if (!constantOn)
    {
      // What char have we got
      if ((c == TBUF_CH_SPACE) || (c == TBUF_CH_EOL))
      {
        // Have we read a token
        if (tokenLen)
        {
          return (TR_OK);
        }

        // Ignore this character
        continue;
      }

      // Check for punctuation
      if (CharType(c) == PUNCTUATION)
      {
        // Haven't read anything yet
        if (!tokenLen)
        {
          sPtr[0] = c;
          sPtr[1] = '\0';
          return(TR_PUN);
        }

        // Successfully read a token
        StepBack();
        return(TR_OK);
      }
    }

    // Add this char to the current token
    if (tokenLen < TBUF_MAX_TOKEN - 1)
    {
      sPtr[tokenLen++] = c;
      sPtr[tokenLen] = '\0';
    }
    else
    {
      TokenError("maximum token length [%d bytes] exceeded", TBUF_MAX_TOKEN-1);
    }
  }
}


// 
// TBuf::PeekToken
//
// Gets the next token and places it in 'peekToken'. The
// next call to NextToken will just get this peek value
//
TBufResult TBuf::PeekToken()
{
  // Only get next token if we haven't peeked already
  if (peekState != PS_ON)
  {
    // Signal pre-peek state
    peekState = PS_PRE;

    // Get the next token
    peekVal = NextToken();
  }

  // Return saved value
  return (peekVal);
}



//
// TBuf::AcceptIdent
//
// Calls NextToken, and generates error if punctuation or EOF
//
void TBuf::AcceptIdent()
{
  // Get next token
  switch (NextToken())
  { 
    case TR_OK  :
      break;

    case TR_PUN :
      ExpectError("identifier", lastToken);
      break;

    case TR_EOF :
      EofError("identifier");
      break;
  }
}


//
// TBuf::AcceptPunct
//
// Calls NextToken, and generates error if not punctuation
//
void TBuf::AcceptPunct()
{
  // Get next token
  switch (NextToken())
  { 
    case TR_OK  :
      ExpectError("punctuation", lastToken);

    case TR_PUN :
      break;

    case TR_EOF :
      EofError("punctuation");
  }
}


//
// TBuf::Accept
//
// Accept token 'accept', or generate error if different
//
void TBuf::Accept(const char *accept)
{
  ASSERT(accept);
  
  // get next token
  switch (NextToken())
  { 
    case TR_OK  :
    case TR_PUN :
      if (Utils::Strcmp(accept, lastToken))
      {
        TokenError("Expecting '%s' but found '%s'", accept, lastToken);
      }
      break;

    case TR_EOF :
      EofError(accept);
  }
}


//
// TBuf::Expect
//
// Generates an error if lastToken is not 'expect'
//
void TBuf::Expect(const char *expect)
{
  ASSERT(expect);
  
  // check last read token
  if (Utils::Strcmp(expect, lastToken))
  {
    TokenError("Expecting '%s' but found '%s'", expect, lastToken);
  }
}


//
// TBuf::ReadConstant
//
// Start reading a constant, stopping on character 'c'
//
void TBuf::ReadConstant(U8 c)
{
  constantOn = TRUE;
  constantChar = c;
}


//
// TBuf::SetCharType
//
// Sets the type of each char in 'pStr' to 'type'
// File does not need to be open to use this function
//
void TBuf::SetCharType(const char *pStr, TBufCharType type)
{
  // step through char array
  for (const char *p = pStr; *p; p++)
  {
    pArray[*p] = type;
  }
}


//
// TBuf::ResetCharTypes
//
// Resets the types of all chars to normal
// File does not need to be open to use this function
//
void TBuf::ResetCharTypes()
{
  Utils::Memset(pArray, NORMAL, sizeof(pArray));
}


// 
// TBuf::NextChar()
//
// Get the next char from the buffer, returning FALSE
// when there is no data left.
//
Bool TBuf::NextChar(U8 *c)
{
  ASSERT(c);

  U8 n;
    
  // Filter comments and carriage returns
  do
  {
    ASSERT(bufPos <= bufSize);

    // Check if at end of data
    if (bufPos == bufSize)
    {
      // Are we reading a string constant
      if (constantOn)
      {
        TokenError("reached end of data in string constant");
      }

      return (FALSE);
    }

    // Get the next character
    n = (U8) bufData[bufPos++];

    // Adjust parse position
    if (n == TBUF_CH_EOL)
    {
      // Are we reading a string constant
      if (constantOn)
      {
        TokenError("newline in string constant");
      }
    
      lastXPos = xPos;
      xPos = 0;
      yPos++;
    }
    else if (n != TBUF_CH_CR)
    {
      xPos++;
    }

    // if not in a constant, check commenting
    if (!constantOn)
    {
      // turn line comment mode on
      if (!commentOn && (CharType(n) == COMMENT))
      { 
        commentOn = TRUE;
      }

      // turn line comment mode off
      if (commentOn && (n == TBUF_CH_EOL))
      {
        commentOn = FALSE;
      }
    }
  }
  while(commentOn || (n == TBUF_CH_CR));

  // convert tabs to spaces
  if (n == TBUF_CH_TAB)
  {
    n = TBUF_CH_SPACE;
  }

  // set the char value
  *c = n;
  return(TRUE);
}


// 
// TBuf::StepBack()
//
// Steps back to the previous char in the buffer.
// NextChar must be called between successive calls
//
void TBuf::StepBack()
{
  // Make sure we've progressed
  if (bufPos)
  {
    // Step back to previous char
    bufPos--;

    // Adjust parse position
    if (bufData[bufPos] == TBUF_CH_EOL)
    {
      xPos = lastXPos;
      yPos--;
    }
    else
    {
      xPos--;
    }
  }
}


// 
// TBuf::CharType
//
// Returns the type of the char 'c'
//
TBuf::TBufCharType TBuf::CharType(U8 c)
{
  return (pArray[c]);
}


//
// TBuf::ExpectError
//
// Triggers error "Expecting %s but found '%s'
//
void TBuf::ExpectError(const char *expected, const char *found)
{
  TokenError("Expected %s but found '%s'", expected, found);
}


//
// TBuf::EofError
//
// Triggers error "Expecting %s but reached end of data"
//
void TBuf::EofError(const char *expected)
{
  TokenError("Expected %s but reached end of data", expected);
}


// 
// TBuf::TokenError
//
// Triggers a fatal parse error with context info
// (must not return from this function)
//
void CDECL TBuf::TokenError(const char *fmt, ...)
{
  ASSERT(errHandler);

  // Process the variable args
  va_list args;
  char fmtBuf[1024];
  va_start(args, fmt);
  vsprintf(fmtBuf, fmt, args);
  va_end(args);

  // Call error handler
  errHandler(bufName.str, xPos, yPos, fmtBuf);
}


//
// TBuf::DefaultError
//
// Fatal error handler
//
void FASTCALL TBuf::DefaultError(const char *bufName, U32 x, U32 y, const char *errStr)
{
  // Trigger the error
  ERR_CONFIG(("%s(%d): Error! (col %d) %s", bufName, y, x, errStr));
}