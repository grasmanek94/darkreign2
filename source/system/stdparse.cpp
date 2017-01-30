///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Standard Token Buffer Parsing Routines
//
// 24-NOV-1997
//

#include "stdparse.h"


///////////////////////////////////////////////////////////////////////////////
//
//  Class StdParse - Contains standard TBuf parsing routines
//


//
// StdParse::ParseNumericVNode
//
// Parses a numeric VNode, NULL if error
//
VNode* StdParse::ParseNumericVNode(TBuf *tBuf)
{
  // Positive by default
  U8 sign = '+';

  // Accept next token
  tBuf->NextToken();

  // Point to dest buffer
  char *buf = tBuf->lastToken;

  // Process sign
  if ((*buf == '-') || (*buf == '+'))
  {
    // Save sign
    sign = *buf;

    // Get numeric value
    switch (tBuf->NextToken())
    {
      // This is what we require
      case TR_OK:
        break;

      // No more tokens, go with the first one
      case TR_EOF:
      case TR_PUN:
        tBuf->TokenError("Numeric value required");

      default:
        ERR_FATAL(("Missing case"));
    }
  }

  // Must have an identifier here
  ASSERT(Utils::Strlen(buf));

  // Allocate a new node
  VNode *newNode = new VNode; 

  // Set the negate flag
  Bool negate = (sign == '-') ? TRUE : FALSE;

  // Calculate the length of the buf
  int len = Utils::Strlen(buf);

  // Is there are standard modifier on the end ?
  if (len > 1)
  {
    switch (buf[len - 1])
    {
      case '%': // percentage
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-2f);
        return (newNode);
        break;

      case 'k': // kilo
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E3f);
        return (newNode);
        break;

      case 'M': // Mega
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E6f);
        return (newNode);
        break;

      case 'G': // Giga
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E9f);
        return (newNode);
        break;

      case 'T': // Tera
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E12f);
        return (newNode);
        break;

      case 'd': // deci
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-1f);
        return (newNode);
        break;

      case 'c': // centi
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-2f);
        return (newNode);
        break;

      case 'm': // milli
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-3f);
        return (newNode);
        break;

      case 'u': // micro
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-6f);
        return (newNode);
        break;

      case 'n': // nano
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-9f);
        return (newNode);
        break;

      case 'p': // pico
        newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1) * 1E-12f);
        return (newNode);
        break;

      default:
        break;
    }
  }

  // Is this a floating point value
  if (Utils::Strchr(buf, '.'))
  {
    newNode->SetupFPoint((F32) Utils::AtoF(buf) * (negate ? -1:1));
  }

  else
  {
    int val = 0;

    // Is this a binary number
    if ((len > 1) && toupper(buf[len-1]) == 'B')
    {
      // Binary integer
      for (int digit = 0; digit < len-1; digit++)
      {
        char c = buf[len-digit-2];

        if (c == '1')
        {
          val += 1<<digit;
        }
        else if (c != '0')
        {
          tBuf->TokenError("Invalid character in binary number");
        }
      }
    }
    else 

    // Is this a hexadecimal number
    if ((len > 2) && (toupper(buf[1]) == 'X'))
    {
      for (int digit = 0; digit < len-2; digit++)
      {
        char c = buf[len-digit-1];

        if (isxdigit(c))
        {
          val += (isdigit(c) ? c-'0' : toupper(c)-'A'+10) << (digit * 4);
        }
        else
        {
          tBuf->TokenError("Invalid character in hexadecimal number");
        }
      }
    }
    else
    {
      // Must be a decimal value
      if (!isdigit(*buf))
      {
        tBuf->TokenError("Invalid numeric value");
      }

      // Convert to integer
      val = Utils::AtoI(buf);
    }

    // Setup node as integer
    newNode->SetupInteger(val * (negate ? -1:1));
  }

  // return the new VNode
  return (newNode);
}


//
// StdParse::ParseStringVNode
//
// Parses a string VNode, NULL if error
//
VNode* StdParse::ParseStringVNode(TBuf *tBuf)
{
  // Allow empty strings
  Bool empty = FALSE;

  // Store string delimiter
  char delim[2];

  // Accept the start of the string literal
  tBuf->NextToken();

  // Insert string delimiter
  delim[0] = *tBuf->lastToken;

  // Insert null terminator
  delim[1] = '\0';

  // Start reading the string constant
  tBuf->ReadConstant(*delim);

  // Read in string
  switch (tBuf->NextToken())
  {
    // Copy string, then accept closing symbol
    case TR_OK:
      tBuf->Accept(delim);
      break;

    // Must have been an empty string
    case TR_PUN:
      tBuf->Expect(delim);
      empty = TRUE;
      break;

    // This should be caught in tBuf
    case TR_EOF:
      ERR_FATAL(("Unexpected return code"));

    default:
      ERR_FATAL(("Missing case"));
  }

  // Create the new VNode
  VNode *newNode = new VNode;
  newNode->SetupString(empty ? "" : tBuf->prevToken);
  return (newNode);
}


//
// StdParse::ParseAtomicVNode
//
// Parse atomic VNode data from 'tBuf', or return NULL if not valid
//
VNode* StdParse::ParseAtomicVNode(TBuf *tBuf)
{
  VNode *newNode = 0;

  // Peek at the next token
  switch (tBuf->PeekToken())
  {
    case TR_OK:
    {
      // Numeric value
      if (isdigit(*tBuf->peekToken) || (*tBuf->peekToken == '.'))
      {
        newNode = ParseNumericVNode(tBuf);
      }
      break;
    }

    case TR_PUN:
    {
      switch (*tBuf->peekToken)
      {
        // String value
        case '"':
        case '\'':
          newNode = ParseStringVNode(tBuf);
          break;

        // Numeric value
        case '-':
        case '+':
          newNode = ParseNumericVNode(tBuf);
          break;       
      }
      break;
    }

    // reached the end of the data
    case TR_EOF:
      break;

    default:
      ERR_FATAL(("Missing case"));
  }

  // Return the new VNode
  return (newNode);
};
