///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Multi Language Support
//
// 06-JAN-99
//


#ifndef __MULTILANGUAGE_H
#define __MULTILANGUAGE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32.h"
#include "varsys.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define MULTILANGUAGE_MAXDATA 16384
#define MULTILANGUAGE_MAXKEY  128

typedef StrBuf<MULTILANGUAGE_MAXKEY> MultiIdent;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiLanguage
//
namespace MultiLanguage
{

  // Initialization and Shutdown
  void Init();
  void Done();

  // Configuration
  void Configure(FScope *fScope);

  // Lookup (note that size is in BYTES not CHARS!)
  void Lookup(const char *path, const CH *&data, U32 &size, U32 &version);

  // Lookup (note that size is in BYTES not CHARS!)
  void Lookup(const char *path, const CH *&data, U32 &size);

  // SetData
  void SetData(const char *path, const CH *data, U32 size);

  // IsKeyOutOfDate
  Bool IsKeyOutOfDate(const char *path);

  // SetLanguage
  void SetLanguage(const char *language);

  // SetLanguage
  void SetLanguage(int index);

  // SetBaseLanguage
  void SetBaseLanguage(const char *language);

  // SetBaseLanguage
  void SetBaseLanguage(int index);

  // GetLanguage
  const char *GetLanguage();

  // GetLanguageIndex
  int GetLanguageIndex();

  // GetBaseLanguage
  const char *GetBaseLanguage();

  // GetBaseLanguageIndex
  int GetBaseLanguageIndex();

  // GetNumLanguages
  U32 GetNumLanguages();

  // SetFont
  void SetFont(LOGFONT *logFont);

  // GetFont
  LOGFONT *GetFont();

  // GetFirstLangage
  const char *GetFirstLanguage();

  // GetNextLanguage
  const char *GetNextLanguage();

  // Set Path
  void SetPath(const char *path);

  // Create a new key
  VarSys::VarItem *CreateKey(const char *path);

  // Create a new scope
  VarSys::VarItem *CreateScope(const char *path);

  // Delete a key
  void DeleteKey(const char *path);

  // Delete a scope
  void DeleteScope(const char *path);

  // Validate a name
  Bool ValidateName(const char *path, const char *name);

  // Is there renaming danger ?
  Bool RenameDanger(const char *path);

  // Rename a key
  VarSys::VarItem * RenameKey(const char *path, const char *name);

  // Rename a scope
  VarSys::VarItem * RenameScope(const char *path, const char *name);

  // Load Database
  void Load(const char *language);
  
  // Save Database(s)
  void Save();

  // Build a lookup key
  const char * CDECL BuildKey(U32 params, ...);

}


#endif
