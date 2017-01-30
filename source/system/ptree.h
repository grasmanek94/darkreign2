///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Text File Parse Tree Management
//
// 24-NOV-1997
//

#ifndef __PTREE_H
#define __PTREE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tbuf.h"
#include "fscope.h"
#include "file.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// size of parsing work buffer
#define PTREE_WORKBUF_LEN       256

// punctuation characters 
#define PTREE_PUNCTUATION       "#(,');{}=-+&\""

// line comment characters
#define PTREE_COMMENTS          "/"

// identifies a ptree binary file
#define PTREE_ID      (*((U32*)"LARD"))

// version 1 id
#define PTREE_VER001   0x00000001


// Forward declaration
namespace FileSys
{
  class DataFile;
}

class BlockFile;


///////////////////////////////////////////////////////////////////////////////
//
//  Class PTree
//
//  Parses text files of a set format and builds a parse tree.
//

#pragma pack(push, 1)

class PTree
{ 
public:

  // Binary file information
  struct BinaryInfo
  {
    U32 fScopeSize;
    U32 symbolSize;
    U32 symbolCount;
  };

private:

  // binary file keys
  enum 
  {
    BK_ARGSTRING,
    BK_ARGINTEGER,
    BK_ARGFLOAT
  };

  // crc all the files added to the ptree
  Bool useCrc;
  U32 crc;

  // last error string
  char lastError[256];

  // global scope
  FScope gScope;

  // current token buffer being processed, on top of file stack
  TBuf *tBuf;

  // List of symbols, indexed by crc
  BinTree<const char> symbols;

  // parse a text file, adding to 'fScope'
  Bool AddFileToScope(const char *fName, FScope *fScope);

  // file syntax parsing functions
  TBufResult ParseFunctionContents(FScope *fScope);
  void ParseFunctionConstruct(FScope *fScope);
  void ParseDirective(FScope *fScope);
  FScope* ParseFunctionArguments(FScope *fScope);
  void ParseVariable(FScope *fScope);
  VNode* ParseVNodeData(FScope *fScope);
  VNode* ParseNumericVNode(FScope *fScope);
  VNode* ParseStringVNode(FScope *fScope);
  void ParseEnumeration(FScope *fScope);
  VNode* ParseVariableReference(FScope *fScope);
  VNode* ParsePointerVNode(FScope *fScope);
  void PeekPunctuation();
  void CheckLegalIdent(const char *ident);

  // for saving trees as text
  void Indent(File &file, U32 i);
  void NextLine(File &file);
  void WriteString(File &file, const char *str);
  void WriteFunctionContents(File &file, FScope *fScope, U32 indent);

  // for saving trees as binary
  char *binaryData;
  char *binaryPos;
  U32 binaryRemaining;
  void ReadBinaryData(void *dest, U32 size);
  void BinWriteFunctionContents(BlockFile *file, FScope *fScope);
  void BinReadFunctionContents(FScope *fScope);
  void BinWriteSymbols(BlockFile *file, BinTree<const char> &symbols);
  void BinReadSymbols(BinTree<const char> &symbols);

  // temporary buffer available for parsing functions
  char workBuf[PTREE_WORKBUF_LEN];
  
public:

  PTree(Bool useCrc = FALSE, U32 crc = 0);
  ~PTree();

  // parse a text file, adding to the global scope
  Bool AddFile(const char *fName);

  // returns a pointer to the global scope, and resets all iterators within it.
  FScope * GetGlobalScope();

  // save tree as text to 'fName'
  Bool WriteTreeText(const char *fName);

  // save tree as binary to 'fName'
  Bool WriteTreeBinary(const char *fName, Bool symbols = TRUE, BinaryInfo *info = NULL);

  // return the last error string
  const char * LastError() 
  { 
    return lastError; 
  }

  // get the crc
  U32 GetCrc()
  {
    return (crc);
  }

};

#pragma pack(pop)

#endif
