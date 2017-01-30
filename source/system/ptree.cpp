///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Text File Parse Tree Management
//
// 24-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"
#include "ptree.h"
#include "stdparse.h"
#include "blockfile.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class PTree
//

// Version block identifier
static const char *VersionBlock = "PTree Binary File";

// FScope block identifier
static const char *FScopeBlock = "FScope Data";

// Symbol block identifer
static const char *SymbolBlock = "Symbol Data";

// Current PTree binary version
static const U32 Version = 1;


//
// Store all unique symbols in a tree
//
static void BuildSymbols(BinTree<const char> &sym, FScope *fScope)
{
  VNode *bNode;

  // reset the iterators
  fScope->InitIterators();

  // write contents
  while ((bNode = fScope->NextBodyVNode()) != NULL)
  {
    // Descend into sub scopes
    if (bNode->nType == VNode::NT_SCOPE)
    {
      FScope *sScope = bNode->GetScope();

      U32 crc = sScope->NameCrc();

      // Add unique symbols to the tree
      const char *found = sym.Find(crc);

      if (!found)
      {
        const char *str = sScope->NameStr();
        sym.Add(crc, str);
      }

#ifdef DEVELOPMENT
      else
      {
        if (Utils::Stricmp(found, sScope->NameStr()))
        {
          ERR_FATAL(("Uh-oh Crc 0x%.8X for '%s' and '%s'", crc, found, sScope->NameStr()))
        }
      }
#endif

      BuildSymbols(sym, sScope);
    }
  }
}


//
// Constructor
//
PTree::PTree(Bool useCrc, U32 crc) 
: gScope(NULL, "GlobalScope"), 
  tBuf(NULL), 
  binaryData(NULL),
  useCrc(useCrc),
  crc(crc)
{
  // Clear last error string
  *lastError = '\0';
}


//
// Destructor
//
PTree::~PTree()
{
  // Clear stack
  while (tBuf)
  {
    TBuf *next = tBuf->listNext;
    delete tBuf;
    tBuf = next;
  }
}


// 
// PTree::AddFileToScope
//
// Reads 'fName', adding to the scope 'fScope'.
// False if an error occured, use LastError
//
Bool PTree::AddFileToScope(const char *fName, FScope *fScope)
{
  ASSERT(fName);
  ASSERT(fScope);

  // Binary block file
  BlockFile bFile;
  Bool binaryOk = FALSE;
  
  // Try and read binary file first
  if (bFile.Open(fName, BlockFile::READ, FALSE))
  {
    // Is this a supported version
    if (bFile.OpenBlock(VersionBlock))
    {
      U32 ver;
      bFile.ReadFromBlock(&ver, 4);
      bFile.CloseBlock();

      switch (ver)
      {
        case 1:
          binaryOk = TRUE;
          break;
      }
    }
  }

  if (binaryOk)
  {
    U32 size;

    // Read symbol data
    if (bFile.OpenBlock(SymbolBlock, FALSE, &size))
    {
      binaryData = binaryPos = (char *)bFile.GetBlockPtr();
      binaryRemaining = size;

      BinReadSymbols(symbols);
      bFile.CloseBlock();
    }

    // Read fscope data
    if (bFile.OpenBlock(FScopeBlock, FALSE, &size))
    {
      binaryData = binaryPos = (char *)bFile.GetBlockPtr();
      binaryRemaining = size;

      if (useCrc)
      {
        crc = Crc::Calc((char *)bFile.GetBlockPtr(), size, crc);
      }

      BinReadFunctionContents(fScope);
      bFile.CloseBlock();
    }

    // Delete all symbols
    symbols.UnlinkAll();

    bFile.CloseBlock();
    bFile.Close();
  }
  else
  {
    // handle to file from file system
    FileSys::DataFile *file = FileSys::Open(fName);

    // didn't find the file
    if (!file)
    {
      Utils::Sprintf(lastError, 256, "Unable to open file '%s'", fName);
      return (FALSE);
    }

    // get the size of the file
    U32 fileSize = file->Size();

    // ignore if size is zero
    if (!fileSize)
    {
      FileSys::Close(file);
      return (TRUE);
    }

    if (useCrc)
    {
      crc = Crc::Calc(file->GetMemoryPtr(), file->Size(), crc);
    }

    // Get poitner to start of file
    char *fileData = (char *)file->GetMemoryPtr();

    // allocate a new token reader
    TBuf *newTBuf = new TBuf;

    // place on top of stack
    newTBuf->listNext = tBuf;
    tBuf = newTBuf;

    // setup buffer
    tBuf->Setup(fileData, fileSize, fName);

    // setup character types
    tBuf->SetCharType(PTREE_PUNCTUATION, TBuf::PUNCTUATION);
    tBuf->SetCharType(PTREE_COMMENTS, TBuf::COMMENT);

    // add contents to given scope
    while (ParseFunctionContents(fScope) != TR_EOF);

    // close the file
    tBuf->Done();

    // remove from stack
    newTBuf = tBuf;
    tBuf = tBuf->listNext;
    delete newTBuf;

    // close the file
    FileSys::Close(file);
  }

  // success
  return (TRUE);
}


// 
// PTree::AddFile
//
// Reads 'fName', adding to the global scope
// False if an error occured, use LastError
//
Bool PTree::AddFile(const char *fName)
{
  return (AddFileToScope(fName, &gScope));
}

  
//
// PTree::GetGlobalScope
//
// initialises the iterators and returns a pointer to the global scope
//
FScope* PTree::GetGlobalScope()
{
  gScope.InitIterators();
  return (&gScope);
}


//
// PTree::PeekPunctuation
//
// peeks at the next token and confirms that it is a
// punctuation character.
//
void PTree::PeekPunctuation()
{
  switch (tBuf->PeekToken())
  {
    case TR_OK:
      tBuf->ExpectError("punctuation", tBuf->peekToken);

    case TR_EOF:
      tBuf->EofError("punctuation");

    case TR_PUN:
      break;

    default:
      ERR_FATAL(("Missing case"));
  } 
}


// 
// PTree::CheckLegalIdent
//
// checks to make sure 'ident' is a legal identifier name
//
void PTree::CheckLegalIdent(const char *ident)
{
  if (!FScope::IsLegalIdent(ident))
  {
    tBuf->TokenError("Illegal identifier name '%s'", ident);
  }
}


//
// PTree::ParseFunctionContents
//
// parses the contents of a function, assuming either a "{" has been 
// read, or at start of file. adds to the scope pointed to by 'fScope'.
//
// returns :
//
// TR_OK - the finishing '}' was read
// TR_EOF - end of file was reached
//
TBufResult PTree::ParseFunctionContents(FScope *fScope)
{
  ASSERT(fScope);

  // parse contents until closing brace or eof
  for (;;)
  {
    // peek at the next token
    switch (tBuf->PeekToken())
    {
      // some construct
      case TR_OK:
        ParseFunctionConstruct(fScope);
        break;
      
      // some form of punctuation      
      case TR_PUN:       
        switch (*tBuf->peekToken)
        {
          // do pre-processor directive
          case '#':
            ParseDirective(fScope);
            break;

          // only other option is to close function
          default:
            tBuf->Accept("}");
            return (TR_OK);
        }
        break;

      // reached the end of the file
      case TR_EOF:
        return (TR_EOF);

      default:
        ERR_FATAL(("Missing case"));
    }    
  }
}


//
// PTree::ParseFunctionConstruct
//
// parses a function construct that starts with an identifier.
// eg. FunctionName(); or var = 5;  returns once that construct 
// has been completely parsed.
//
void PTree::ParseFunctionConstruct(FScope *fScope)
{
  ASSERT(fScope);

  // accept the identifier
  tBuf->AcceptIdent();

  // peek at punctuation
  PeekPunctuation();

  // do specific parsing operations
  switch (*tBuf->peekToken)
  {
    // variable assignment
    case '=':
      ParseVariable(fScope);
      break;

    // sub-function
    case '(':
    {
      // make sure legal function name
      CheckLegalIdent(tBuf->lastToken);
      
      // parse the function arguments
      FScope *newScope = ParseFunctionArguments(fScope);

      // accept '{' or ';'
      if (tBuf->NextToken() == TR_PUN)
      {
        switch (*tBuf->lastToken)
        {
          // parse function body
          case '{':

            // require a '}' to be found
            if (ParseFunctionContents(newScope) == TR_EOF)
            {
              tBuf->EofError("'}'"); 
            }
            break;

          // no function body
          case ';':
            break;

          default:
            tBuf->ExpectError("'{' or ';'", tBuf->lastToken);
        }
      }
      else
      {
        tBuf->ExpectError("'{' or ';'", tBuf->lastToken);
      }
      break;
    }
        
    // enumeration
    case '{':
      if (Utils::Strcmp(tBuf->lastToken, "enum"))
      {
        tBuf->ExpectError("enum", tBuf->lastToken);
      }
      ParseEnumeration(fScope);
      break;

    default:
      tBuf->ExpectError("function scope construct", tBuf->peekToken);
  }
}


//
// PTree::ParseDirective
//
// parses and executes a pre-processor directive, assumes the
// next token will be '#'.  'fScope' is the scope in which the 
// command is being executed.
// 
void PTree::ParseDirective(FScope *fScope)
{
  // accept the directive char
  tBuf->Accept("#");

  // accept the directive identifier
  tBuf->AcceptIdent();

  // do specific operation
  switch (Crc::CalcStr(tBuf->lastToken))
  {
    case 0x8CBCE90A: // "include"
    {
      tBuf->Accept("\"");
      tBuf->ReadConstant('"');
      tBuf->AcceptIdent();
      tBuf->Accept("\"");

      // add the file to the current scope
      if (!AddFileToScope(tBuf->prevToken, fScope))
      {
        tBuf->TokenError("#include error : %s", LastError());
      }
      break;
    }

    case 0x6282142B: // "log"
    {
      tBuf->Accept("\"");
      tBuf->ReadConstant('"');
      tBuf->AcceptIdent();
      tBuf->Accept("\"");
      LOG_DIAG(("%s(%d): %s", tBuf->BufferName(), tBuf->CurrentLine(), tBuf->prevToken));
      break;
    }
  
    case 0x8D39DDCB: // "dumpvar"
    {
      tBuf->Accept("(");
      tBuf->AcceptIdent();
      
      U32 crc = Crc::CalcStr(tBuf->lastToken);
      VNode *varNode = fScope->FindVariableInScope(crc);

      LOG_DIAG
      ((
        "%s(%d): #dumpvar(%s) = %s", tBuf->BufferName(), tBuf->CurrentLine(),
        tBuf->lastToken, (varNode) ? varNode->StringForm() : "NOT FOUND!"
      ));

      tBuf->Accept(")");
      break;
    }

    default:
      tBuf->TokenError("Unknown pre-processor directive '%s'", tBuf->lastToken);
  }
}


// 
// PTree::ParseFunctionArguments
//
// assumes "FuncName" has just been parsed, and that '(' is the next
// token.  creates a new function scope and reads the argument list.
// 'fScope' is the enclosing scope. returns a pointer to a new function
// scope when the closing ')' has been read.
//
FScope* PTree::ParseFunctionArguments(FScope *fScope)
{
  ASSERT(fScope);
 
  // allocate a new function scope
  FScope *newScope = new FScope(fScope, tBuf->lastToken);

  // new argument value
  VNode *newNode;

  // do we require an argument
  Bool reqArg = FALSE;

  // continue until closing bracket is read
  do
  {
    // accept '(' or ','
    tBuf->AcceptPunct();

    // add a new argument
    newNode = ParseVNodeData(newScope);

    if (newNode)
    {
      // add to function argument list
      newScope->argList.Append(newNode);
    }
    else
    {
      // did we require an argument
      if (reqArg)
      {
        tBuf->ExpectError("function argument", tBuf->lastToken);
      }
    }
    
    // if continue from here, require argument, eg. "5,)"
    reqArg = TRUE;

  } while (*tBuf->peekToken == ',');
        
  // must be closing bracket
  tBuf->Accept(")");

  // return the new function scope
  return (newScope);
}


//
// PTree::ParseVariable
//
// assumes 'varName' has just been parsed, and that the
// next token is '='.  creates a new VNode global variable 
// within the scope 'fScope'.  returns when the terminating ';'
// char is read.
//
void PTree::ParseVariable(FScope *fScope)
{
  TokenStr varName;
  VNode *varNode, *valNode;

  // save the variable name
  Utils::Strcpy(varName, tBuf->lastToken);

  // make sure legal variable name
  CheckLegalIdent(varName);

  // accept the equals
  tBuf->Accept("=");

  // get the assigned value
  valNode = ParseVNodeData(fScope);

  if (valNode)
  {
    // calculate the crc of our identifier
    U32 crc = Crc::CalcStr(varName);

    // are we just changing the value of an existing variable
    varNode = fScope->FindVariableInScope(crc);

    if (varNode)
    {
      // make sure atomic type hasn't changed
      if (valNode->aType != varNode->aType)
      {
        tBuf->TokenError("Atomic type of '%s' is different to previous definition", varName);
      }

      // clear the variable VNode
      varNode->Clear();

      // and setup again
      varNode->SetupVariable(varName, valNode);
    }
    else
    {
      // create a variable node and attach value node
      varNode = new VNode;
      varNode->SetupVariable(varName, valNode);

      // add to global vars within scope
      fScope->bodyList.Append(varNode);
    }
  }
  else
  {
    tBuf->ExpectError("variable value", tBuf->lastToken);
  }

  // accept the end of statement
  tBuf->Accept(";");
}


//
// PTree::ParseVNodeData
//
// Parses the data for a VNode. 'fScope' points to the function scope 
// in which the data is being read (used for variable references).
// returns pointer to a new VNode or null if none was found.  Next char 
// is peeked at and confirmed to be punctuation.
//
VNode* PTree::ParseVNodeData(FScope *fScope)
{
  // Parse numeric or string value
  VNode *newNode = StdParse::ParseAtomicVNode(tBuf);

  // Were we successful
  if (newNode)
  {
    PeekPunctuation();
  }
  else
  {
    // Peek at the token in question
    switch (tBuf->PeekToken())
    {
      case TR_OK:
      {
        // Must be a variable reference
        newNode = ParseVariableReference(fScope);
        break;
      }

      case TR_PUN:
      {
        // Pointer to existing variable
        if (*tBuf->peekToken == '&')
        {
          newNode = ParsePointerVNode(fScope);
        }
        break;
      }

      // Reached the end of the file
      case TR_EOF:
        tBuf->EofError("data value");

      default:
        ERR_FATAL(("Missing case"));
    }
  }

  // return the new VNode
  return (newNode);
}


//
// PTree::ParseEnumeration
//
// parse an enumeration 'enum { one=1, two, three };'
// 'fScope' is the scope in which the enumeration is being parsed.
//
void PTree::ParseEnumeration(FScope *fScope)
{
  S32 incVal = 0;
  VNode *varNode, *valNode, *newNode;
  TokenStr ident;

  // accept out open brace
  tBuf->Accept("{");

  // continue until closing brace is read
  do
  {   
    // accept an identifier
    tBuf->AcceptIdent();

    // save the identifier
    Utils::Strcpy(ident, tBuf->lastToken);

    // make sure legal ident
    CheckLegalIdent(ident);

    // accept either '=' or ',' or '}' 
    tBuf->AcceptPunct();
    
    // is there a value assignment
    if (*tBuf->lastToken == '=')
    {
      // read the value
      newNode = ParseVNodeData(fScope);

      if (newNode)
      {
        if (newNode->aType == VNode::AT_INTEGER)
        {
          // adjust the increment value
          incVal = newNode->GetInteger();

          // delete the node
          delete newNode;

          // accept either ',' or '}'
          tBuf->AcceptPunct();
        }
        else
        {
          tBuf->ExpectError("integer value", tBuf->prevToken);
        }
      }
      else
      {
        tBuf->ExpectError("integer value", tBuf->lastToken);
      }
    }

    // allocate the new nodes
    varNode = new VNode;
    valNode = new VNode;

    // setup the value
    valNode->SetupInteger(incVal++);

    // setup the variable
    varNode->SetupVariable(ident, valNode);

    // add to global variable list
    fScope->bodyList.Append(varNode);
  } 
  while (*tBuf->lastToken == ',');
        
  // must be closing brace
  tBuf->Expect("}"); 

  // and finish with a semi-colon
  tBuf->Accept(";");
}


//
// PTree::ParseVariableReference
//
// searches for the next token as a variable visible from the current 
// scope.  returns a new VNode with the atomic value of that variable,
// when the next char is peeked at and is punctuation.
//
VNode* PTree::ParseVariableReference(FScope *fScope)
{
  VNode *varNode;

  // accept the variable identifier
  tBuf->AcceptIdent();
  
  // calculate the crc of this string
  U32 crc = Crc::CalcStr(tBuf->lastToken);

  // search for this variable
  varNode = fScope->FindVariableVisible(crc);

  // trigger error if not found
  if (!varNode)
  {
    tBuf->TokenError("'%s' : undeclared identifier", tBuf->lastToken);
  }

  // create a new atomic VNode
  VNode *newNode = varNode->NewAtomicNode();

  // check that we are able to make atomic nodes of this type
  if (!newNode)
  {
    tBuf->TokenError("unable to use variable '%s' in this manner", tBuf->lastToken);
  }

  // confirm next char is punctuation
  PeekPunctuation();

  return (newNode);
}


//
// PTree::ParsePointerVNode
//
// searches for the next token as a variable visible from the current 
// scope. returns a pointer to the new vNode. 'fScope' points to the 
// function scope in which the data is being read.  next char is 
// peeked at and confirmed to be punctuation.
//
VNode* PTree::ParsePointerVNode(FScope *fScope)
{
  VNode *varNode;

  // accept pointer char
  tBuf->Accept("&");

  // accept the variable identifier
  tBuf->AcceptIdent();
  
  // calculate the crc of this string
  U32 crc = Crc::CalcStr(tBuf->lastToken);

  // search for this variable
  varNode = fScope->FindVariableVisible(crc);

  // trigger error if not found
  if (!varNode)
  {
    tBuf->TokenError("'%s' : undeclared identifier", tBuf->lastToken);
  }

  // create a new VNode that points to it
  VNode *exist = new VNode;
  exist->SetupPointer(varNode);

  // confirm punctuation char
  PeekPunctuation();

  return (exist);
}


//
// PTree::NextLine
// 
// write end of line and carriage return
//
void PTree::NextLine(File &file)
{
  file.WriteU8(13);
  file.WriteU8(10);
}


//
// PTree::WriteString
// 
// write a null terminated string to file (without the null)
//
void PTree::WriteString(File &file, const char *str)
{
  file.Write(str, Utils::Strlen(str));
}


//
// PTree::Indent
// 
// writes 'i' spaces to 'file'
//
void PTree::Indent(File &file, U32 i)
{
  for (U32 indent = 0; indent < i; indent++)
  {
    file.WriteU8(' ');
  }
}


//
// PTree::WriteFunctionContents
//
// write a function to a file
//
void PTree::WriteFunctionContents(File &file, FScope *fScope, U32 indent)
{
  ASSERT(fScope);
  VNode *bNode;
  U32 maxVarLen = 0;

  // reset the iterators
  fScope->InitIterators();

  // find largest var for lining them up nicely
  while ((bNode = fScope->NextBodyVNode()) != 0)
  {
    // is this bNode a variable
    if (bNode->nType == VNode::NT_VARIABLE)
    {
      // get length
      U32 len = Utils::Strlen(bNode->GetVariableStr());   
    
      // record if largest
      if (len > maxVarLen)
      {
        maxVarLen = len;
      }
    }
  }

  // reset the iterators
  fScope->InitIterators();

  VNode::VNodeType lastNodeType = VNode::NT_NONE;
  Bool prevBody = FALSE, firstFunc = TRUE;

  // write contents
  while ((bNode = fScope->NextBodyVNode()) != 0)
  {
    switch (bNode->nType)
    {
      // a local variable
      case VNode::NT_VARIABLE :
      {         
        ASSERT(maxVarLen >= Utils::Strlen(bNode->GetVariableStr()));

        if (prevBody)
        {
          NextLine(file);
        }

        Indent(file, indent);
        WriteString(file, bNode->GetVariableStr());
        Indent(file, maxVarLen - Utils::Strlen(bNode->GetVariableStr()));
        WriteString(file, " = ");
        WriteString(file, bNode->StringForm());
        WriteString(file, ";");
        NextLine(file);
        
        break;
      }

      // a sub-function
      case VNode::NT_SCOPE :
      {
        // get the scope
        FScope *sScope = bNode->GetScope();

        // do we need an empty line
        if 
        (
          prevBody || (!firstFunc && sScope->HasBody()) || 
          (lastNodeType == VNode::NT_VARIABLE)
        )
        {
          NextLine(file);
        }
    
        Indent(file, indent);
        WriteString(file, sScope->NameStr());
        WriteString(file, "(");

        // write arguments
        Bool firstArg = TRUE;
        VNode *aNode;

        while ((aNode = sScope->NextArgument()) != 0)
        {
          if (!firstArg)
          {
            WriteString(file, ", ");
          }

          WriteString(file, aNode->StringForm());
          firstArg = FALSE;
        }

        WriteString(file, ")");
 
        // are there any contents
        if (sScope->HasBody())
        {
          NextLine(file);
          Indent(file, indent);
          WriteString(file, "{");
          NextLine(file);

          // write contents
          WriteFunctionContents(file, sScope, indent + 2);

          Indent(file, indent);
          WriteString(file, "}");
          prevBody = TRUE;
        }
        else
        {
          WriteString(file, ";");
          prevBody = FALSE;
        }

        NextLine(file);   
        firstFunc = FALSE;

        break;
      }

      default :
        break;
    }

    // save the type of this bNode
    lastNodeType = bNode->nType;
  }
}


//
// PTree::WriteTreeText
//
// save current tree as text to 'fName'
//
Bool PTree::WriteTreeText(const char *fName)
{
  File file;
  
  if (!file.Open(fName, File::WRITE | File::CREATE))
  {
    LOG_ERR(("Unable to write to file '%s'", fName));
    return (FALSE);
  }
    
  // write header
  WriteString(file, "//////////////////////////////////////////////////////////////////////////");
  NextLine(file);
  WriteString(file, "//");
  NextLine(file);
  WriteString(file, "// Copyright 1998-2000 Pandemic Studios");
  NextLine(file);
  WriteString(file, "//");
  NextLine(file);
  WriteString(file, "// Code-Generated Configuration File");
  NextLine(file);
  WriteString(file, "//");
  NextLine(file);
  NextLine(file);

  // write contents
  WriteFunctionContents(file, GetGlobalScope(), 0);

  file.Close();    

  return (TRUE);
}


//
// BinWriteFunctionContents
//
// Write out the given function
//
void PTree::BinWriteFunctionContents(BlockFile *file, FScope *fScope)
{
  ASSERT(fScope);

  U32 u32;
  U16 u16;
  U8 u8;

  // reset the iterators
  fScope->InitIterators();

  // Get the number of body items
  U32 items = fScope->GetBodyCount();

  // Write it out
  file->WriteToBlock(&items, 4);

  // write contents
  for (U32 i = 0; i < items; i++)
  {
    VNode *bNode = fScope->NextBodyVNode();

    if (!bNode)
    {
      ERR_FATAL(("Expected body node! (%d/%d)", i, items));
    }

    if (bNode->nType != VNode::NT_SCOPE)
    {
      ERR_FATAL(("FScope contained unsupported type!"));
    }

    // get the scope
    FScope *sScope = bNode->GetScope();

    // save the crc of the scope
    u32 = sScope->NameCrc();
    file->WriteToBlock(&u32, 4);

    // Get the number of args
    U32 args = sScope->GetArgCount();

    if (args > U8_MAX)
    {
      ERR_FATAL(("Too many arguments! (%s, %d)", sScope->NameStr(), args));
    }

    // Write it out
    u8 = (U8)args;
    file->WriteToBlock(&u8, 1);

    // write arguments
    for (U32 a = 0; a < args; a++)
    {
      VNode *aNode = sScope->NextArgument();

      if (!aNode)
      {
        ERR_FATAL(("Expected argument! (%d/%d)", a, args));
      }

      switch (aNode->aType)
      {
        case VNode::AT_STRING:
        {
          u8 = (U8)BK_ARGSTRING;
          file->WriteToBlock(&u8, 1);

          // Save out the size
          U32 size = Utils::Strlen(aNode->GetString());
          if (size > U16_MAX)
          {
            ERR_FATAL(("String exceeds max! (%d)", size));
          }

          u16 = (U16)size;
          file->WriteToBlock(&u16, 2);

          // Is there any data
          if (size)
          {
            file->WriteToBlock(aNode->GetString(), size);
          }
          break;
        }

        case VNode::AT_INTEGER:
        {
          u8 = (U8)BK_ARGINTEGER;
          file->WriteToBlock(&u8, 1);

          u32 = aNode->GetInteger();
          file->WriteToBlock(&u32, 4);
          break;
        }

        case VNode::AT_FPOINT:
        {
          u8 = (U8)BK_ARGFLOAT;
          file->WriteToBlock(&u8, 1);

          F32 val = aNode->GetFPoint();
          file->WriteToBlock(&val, sizeof(F32));
          break;
        }

        default:
          ERR_FATAL(("Unsupported type in arg list! (%d)", aNode->aType));
      }
    }

    // write the contents of this subscope
    BinWriteFunctionContents(file, sScope);
  }
}


//
// WriteTreeBinary
//
// save tree as binary to 'fName'
//
Bool PTree::WriteTreeBinary(const char *fName, Bool writeSymbols, PTree::BinaryInfo *info)
{
  BlockFile file;

  if (info)
  {
    Utils::Memset(info, 0, sizeof(BinaryInfo));
  }

  if (!file.Open(fName, BlockFile::CREATE, FALSE))
  {
    LOG_ERR(("Unable to write to file '%s'", fName));
    return (FALSE);
  }

  // Save version block
  if (file.OpenBlock(VersionBlock))
  {
    file.WriteToBlock(&Version, sizeof(Version));      
    file.CloseBlock();
  }
  else
  {
    goto Error;
  }

  // Save tree contents
  if (file.OpenBlock(FScopeBlock))
  {
    BinWriteFunctionContents(&file, GetGlobalScope());
    file.CloseBlock();

    if (info)
    {
      info->fScopeSize = file.SizeOfBlock(Crc::CalcStr(FScopeBlock));
    }
  }
  else
  {
    goto Error;
  }

  // Save symbol info
  if (writeSymbols)
  {
    // Build a list of all symbols
    BuildSymbols(symbols, GetGlobalScope());

    // Write them out
    U32 count;

    if ((count = symbols.GetCount()) > 0)
    {
      if (file.OpenBlock(SymbolBlock))
      {
        BinWriteSymbols(&file, symbols);

        symbols.UnlinkAll();
        file.CloseBlock();

        if (info)
        {
          info->symbolSize = file.SizeOfBlock(Crc::CalcStr(SymbolBlock));
          info->symbolCount = count;
        }
      }
      else
      {
        symbols.UnlinkAll();
        goto Error;
      }
    }
  }

  // Success
  file.Close();
  return (TRUE);

Error:

  // Failed
  file.CloseBlock();
  file.Close();
  File::Unlink(fName);

  LOG_ERR(("Error writing to file '%s'", fName));

  return (FALSE);
}


//
// ReadBinaryData
//
// Read data from the binary chunk in memory
//
void PTree::ReadBinaryData(void *dest, U32 size)
{
  ASSERT(binaryData);

  if (size <= binaryRemaining)
  {
    memcpy(dest, binaryPos, size);
    binaryPos += size;
    binaryRemaining -= size;    
  }
  else
  {
    ERR_FATAL(("Failed reading %d bytes from binary data"));
  }
}


//
// BinReadFunctionContents
//
// Read in the given function
//
void PTree::BinReadFunctionContents(FScope *fScope)
{
  ASSERT(fScope);

  // Read in the number of items in this scope
  U32 items;
  ReadBinaryData(&items, sizeof(items));

  // Read contents
  for (U32 i = 0; i < items; i++)
  {
    // Read the scope crc
    U32 crc;
    ReadBinaryData(&crc, sizeof(crc));

    // Create the new scope
    FScope *newScope = new FScope(fScope, symbols.Find(crc), crc);

    // Get the number of args
    U8 args;
    ReadBinaryData(&args, sizeof(args));

    // write arguments
    for (U32 a = 0; a < args; a++)
    {
      // Allocate a new value node
      VNode *newNode = new VNode;

      // Read the type
      U8 type;
      ReadBinaryData(&type, sizeof(type));

      switch (type)
      {
        case BK_ARGSTRING:
        {
          static char buf[1024];

          U16 size;
          ReadBinaryData(&size, sizeof(size));
          ASSERT(size < 1024);
        
          // Is there any data
          if (size)
          {
            ReadBinaryData(buf, size);
          }
          buf[size] = '\0';

          newNode->SetupString(buf);
          break;
        }

        case VNode::AT_INTEGER:
        {
          U32 val;
          ReadBinaryData(&val, sizeof(val));
          newNode->SetupInteger((S32)val);
          break;
        }

        case VNode::AT_FPOINT:
        {
          F32 val;
          ReadBinaryData(&val, sizeof(val));
          newNode->SetupFPoint(val);
          break;
        }

        default:
          ERR_FATAL(("Unsupported type in binary file!"));
      }

      // Add the new node
      newScope->argList.Append(newNode);
    }

    // write the contents of this subscope
    BinReadFunctionContents(newScope);
  }
}


//
// BinWriteSymbols
//
// Write symbols to block file
//
void PTree::BinWriteSymbols(BlockFile *file, BinTree<const char> &symbols)
{
  U32 u32;

  // Number of symbols
  u32 = symbols.GetCount();
  file->WriteToBlock(&u32, 4);

  for (BinTree<const char>::Iterator i(&symbols); *i; i++)
  {
    const char *s = *i;
    U16 len = U16(Utils::Strlen(s) + 1);

    // Write crc, string length, and null terminated string
    u32 = i.GetKey();

    file->WriteToBlock(&u32, 4);
    file->WriteToBlock(&len, 2);
    file->WriteToBlock(s, len);
  }
}


//
// BinReadSymbols
//
// Read symbols from block file
//
void PTree::BinReadSymbols(BinTree<const char> &symbols)
{
  U32 count;

  // Read symbol count
  ReadBinaryData(&count, 4);

  for (U32 i = 0; i < count; i++)
  {
    U32 crc;
    U16 len;

    ReadBinaryData(&crc, 4);
    ReadBinaryData(&len, 2);

    // Add this string and key
    symbols.Add(crc, (const char *)binaryPos);

    binaryPos += len;
    binaryRemaining -= len;
  }
}
