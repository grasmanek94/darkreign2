///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Multi Language Support
//
// 06-JAN-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multilanguage.h"
#include "file.h"
#include "varsys.h"
#include "ptree.h"
#include "varfile.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiLanguage
//
namespace MultiLanguage
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Language
  //
  struct Language
  {
    // Language name
    GameIdent name;

    // Language index
    int index;

    // Font
    LOGFONT font;

  };

#pragma warning(push)
#pragma warning(disable: 4200)

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Item
  //
  struct Item
  {
    U32 version;
    U16 data[];
  };

#pragma warning(pop)


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialization Flag
  static Bool initialized = FALSE;

  // Currently loaded languages
  static List<Language> languages;

  // Language iterator
  static List<Language>::Iterator languageIterator(&languages);

  // Path to load/save databases to
  static StrBuf<260> dbPath;

  // Current language
  static Language *currentLanguage;

  // Base language
  static Language *baseLanguage;

  // Root scope of multilanguage
  static const char *rootScope = "multilanguage";

  // Default data
  static const U16 *defaultData = { 0x0000 };

  // Default font
  static LOGFONT defaultFont = 
  {
    -13,    // Height
    0,      // Width
    0,      // Escapement
    0,      // Orientation
    400,    // Weight
    0,      // Italic
    0,      // Underline
    0,      // StrikeOut
    0,      // CharSet
    3,      // OutPrecision
    2,      // ClipPrecision
    1,      // Quality
    34,     // PitchAndFamily
    "Arial" // FaceName
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static const char * MakePath(const char *path);
  static const char * MakePath(const char *path, int index);
  static const char * ComposeName(const char *path, const char *name);
  static Item *GetItem(const char *path);
  static Item *GetItem(const char *path, U32 &size);
  static void LoadLanguage(const char *name, const char *file);
  static void Handler(U32 pathCrc);


  //
  // Initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    // Register Scope Handler
    VarSys::RegisterHandler(rootScope, Handler);

    // Reset current and base language
    currentLanguage = NULL;
    baseLanguage = NULL;

    // Set initialized flag
    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Destroy order time scope
    VarSys::DeleteItem(rootScope);

    // Delete languages
    languages.DisposeAll();

    // Reset current and base language
    currentLanguage = NULL;
    baseLanguage = NULL;

    // Clear initialized flag
    initialized = FALSE;
  }


  //
  // Configure
  //
  void Configure(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x971E8A6A: // "LoadLanguage"
          LoadLanguage(StdLoad::TypeString(sScope), NULL);
          break;

        case 0xF8E9DCDB: // "SetLanguage"
          SetLanguage(StdLoad::TypeString(sScope));
          break;
      }

    }

  }


  //
  // Lookup
  //
  void Lookup(const char *path, const CH *&data, U32 &size, U32 &version)
  {
    ASSERT(initialized);

    if (*path)
    {
      Item *item = GetItem(path, size);

      if (item)
      {
        version = item->version;
        data = item->data;
        size -= 4;
        return;
      }
    }

    version = 0;
    data = NULL;
    size = 0;
  }


  //
  // Lookup
  //
  void Lookup(const char *path, const CH *&data, U32 &size)
  {
    ASSERT(initialized);
    U32 version;

    Lookup(path, data, size, version);
  }

  
  //
  // GetData
  //
  const CH * GetData(const char *path)
  {
    ASSERT(initialized)

    Item *item = GetItem(path);
    return (item ? item->data : defaultData);
  }


  //
  // GetVersion
  //
  U32 GetVersion(const char *path)
  {
    ASSERT(initialized)

    Item *item = GetItem(path);
    return (item ? item->version : 0);
  }

  
  //
  // SetData
  //
  void SetData(const char *path, const U16 *data, U32 size)
  {
    ASSERT(initialized)

    // Make the full path from the current language and the path
    const char *fullPath = MakePath(path);

    // Get pointer to the binary var of this type
    VarSys::VarItem *item = VarSys::FindVarItem(fullPath);

    // If the item does not exist then create a new one
    if (!item)
    {
      item = VarSys::CreateBinary(fullPath, size + sizeof (U32));
    }

    // The new version number is determined as follows:
    // If the current language is base then increment the old
    // version number to obtain the new one.  If the current
    // language is not base then set the version to the version of
    // the same key in language base.  Note that it is not possible 
    // to have a key which doesn't exist in base but exists in others.
    U32 version;

    if (currentLanguage->index == GetBaseLanguageIndex())
    {
      // Get the new version of this item
      Item *oldItem = (Item *) item->Binary();
      ASSERT(oldItem)
      version = oldItem->version + 1;
    }
    else
    {
      const char *altPath = MakePath(path, GetBaseLanguageIndex());
      VarSys::VarItem *item = VarSys::FindVarItem(altPath);

      if (!item)
      {
        ERR_FATAL(("'%s' not found in base language", path))
      }

      Item *altItem = (Item *) item->Binary();
      ASSERT(altItem)
      version = altItem->version;
    }

    // Build a new item
    Item *newItem = (Item *) new U8[size + sizeof (U32)];
    newItem->version = version;
    Utils::Memcpy(newItem->data, data, size);

    // Save it
    item->SetBinary(size + sizeof (U32), (U8 *) newItem);

    // Dispose of buffer
    delete newItem;
  }


  //
  // IsKeyOutOfDate
  //
  Bool IsKeyOutOfDate(const char *path)
  {
    // If the current language is the base language 
    // then it is imposible for the key to be out of date
    if (currentLanguage == baseLanguage)
    {
      return (FALSE);
    }

    // Find key in the base language first
    const char *fullPath = MakePath(path, GetBaseLanguageIndex());
    VarSys::VarItem *vItem = VarSys::FindVarItem(fullPath);

    if (vItem)
    {
      Item *item = (Item *) vItem->Binary();
      ASSERT(item)
      U32 currentVersion = item->version;

      // Find the key in the current language
      item = GetItem(path);

      // Is it out of date ?
      return ((!item || (item->version < currentVersion)) ? TRUE : FALSE);
    }
    else
    {
      ERR_FATAL(("Could not find '%s' in base language"))
    }
  }


  //
  // SetLanguage
  //
  void SetLanguage(const char *language)
  {
    ASSERT(initialized)

    // Proceed through the list of languages and look for a match
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      if (!Utils::Strcmp((*i)->name.str, language))
      {
        currentLanguage = *i;
        return;
      }
    }

    ERR_FATAL(("Language '%s' does not exist", language))
  }


  //
  // SetLanguage
  //
  void SetLanguage(int index)
  {
    ASSERT(initialized)

    List<Language>::Iterator i(&languages);
    i.GoTo(index);

    if (*i)
    {
      currentLanguage = *i;
    }
    else
    {
      ERR_FATAL(("Language index %d does not exist"))
    }
  }


  //
  // SetBaseLanguage
  //
  void SetBaseLanguage(const char *language)
  {
    ASSERT(initialized)

    // Proceed through the list of languages and look for a match
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      if (!Utils::Strcmp((*i)->name.str, language))
      {
        baseLanguage = *i;
        return;
      }
    }

    ERR_FATAL(("Language '%s' does not exist", language))
  }


  //
  // SetBaseLanguage
  //
  void SetBaseLanguage(int index)
  {
    ASSERT(initialized)

    List<Language>::Iterator i(&languages);
    i.GoTo(index);

    if (*i)
    {
      baseLanguage = *i;
    }
    else
    {
      ERR_FATAL(("Language index %d does not exist"))
    }
  }


  //
  // GetLanguage
  //
  const char * GetLanguage()
  {
    ASSERT(initialized)
    ASSERT(currentLanguage)

    return (currentLanguage->name.str);
  }


  //
  // GetLanguageIndex
  //
  int GetLanguageIndex()
  {
    ASSERT(initialized)
    ASSERT(currentLanguage)

    return (currentLanguage->index);
  }


  //
  // GetBaseLanguage
  //
  const char *GetBaseLanguage()
  {
    ASSERT(initialized)
    ASSERT(baseLanguage)

    return (baseLanguage->name.str);
  }


  //
  // GetBaseLanguageIndex
  //
  int GetBaseLanguageIndex()
  {
    ASSERT(initialized)
    ASSERT(baseLanguage)

    return (baseLanguage->index);
  }


  //
  // GetNumLanguages
  //
  U32 GetNumLanguages()
  {
    ASSERT(initialized)

    return (languages.GetCount());
  }

  
  //
  // GetFont
  //
  LOGFONT * GetFont()
  {
    ASSERT(initialized)
    ASSERT(currentLanguage)

    return (&currentLanguage->font);
  }


  //
  // GetFirstLangage
  //
  const char *GetFirstLanguage()
  {
    ASSERT(initialized)

    languageIterator.GoToHead();
    return (GetNextLanguage());
  }


  //
  // GetNextLanguage
  //
  const char *GetNextLanguage()
  {
    ASSERT(initialized)

    if (*languageIterator)
    {
      const char *name = (languageIterator++)->name.str;
      return (name);
    }
    else
    {
      return (NULL);
    }
  }


  //
  // SetPath
  //
  void SetPath(const char *path)
  {
    ASSERT(initialized)

    dbPath = path;
  }


  //
  // Create a new key
  //
  VarSys::VarItem *CreateKey(const char *path)
  {
    // Only need to create the new binary var in the base language
    const char *fullPath = MakePath(path, GetBaseLanguageIndex());

    VarSys::VarItem *varItem = VarSys::CreateVar(fullPath, "New Key");
    Item *item = (Item *) new U8[sizeof (U32)];
    item->version = 0;
    varItem->InitBinary(4, (const U8 *) item);
    delete [] item;
    return (varItem);
  }


  //
  // Create a new scope
  //
  VarSys::VarItem *CreateScope(const char *path)
  {
    // Only need to create the new scope in the base language
    const char *fullPath = MakePath(path, GetBaseLanguageIndex());

    VarSys::VarItem *item = VarSys::CreateVar(fullPath, "New Scope");
    item->InitScope();
    return (item);
  }


  //
  // Delete a key
  //
  void DeleteKey(const char *path)
  {
    // Deleting a the key in all languages
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      const char *fullPath = MakePath(path, (*i)->index);
      VarSys::DeleteItem(fullPath);
    }
  }


  //
  // Delete a scope
  //
  void DeleteScope(const char *path)
  {
    // Same as deleting a key
    DeleteKey(path);
  }


  //
  // Validate a name
  //
  Bool ValidateName(const char *path, const char *name)
  {
    // A name is valid if it does not already exist
    const char *fullPath = MakePath(path, GetBaseLanguageIndex());
    
    // Compose the new name
    const char *newName = ComposeName(fullPath, name);

    // Does this name exist ?
    return (VarSys::FindVarItem(newName) ? FALSE : TRUE);
  }


  //
  // Is there renaming danger ?
  //
  // Returns TRUE if the key exists in languages other than the base language
  //
  Bool RenameDanger(const char *path)
  {
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      // Find the old var item
      const char *fullPath = MakePath(path, (*i)->index);
      VarSys::VarItem *oldItem = VarSys::FindVarItem(fullPath);

      // Does it exist ?
      if (oldItem)
      {
        // Is this not the base language ?
        if ((*i)->index != GetBaseLanguageIndex())
        {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  }


  //
  // Rename a key
  //
  // Return TRUE if the key existed in a language other than the base language
  //
  VarSys::VarItem * RenameKey(const char *path, const char *name)
  {
    // For each language, find the given path, copy the 
    // existing key into the new one and then delete the old
    VarSys::VarItem *vItem = NULL; 

    // Remame a the key in all languages
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      // Find the old var item
      const char *fullPath = MakePath(path, (*i)->index);
      VarSys::VarItem *oldItem = VarSys::FindVarItem(fullPath);

      // Does it exist ?
      if (oldItem)
      {
        // Get the new name
        const char *newName = ComposeName(fullPath, name);

        // Copy the item
        VarSys::VarItem *newItem = VarSys::CopyVarItem(newName, oldItem);

        // Delete the old item
        VarSys::DeleteItem(oldItem);

        // Is this the base language ?
        if ((*i)->index == GetBaseLanguageIndex())
        {
          vItem = newItem;
        }
      }
      else
      {
        // It is incorrect for this item to
        // not exist in the base language
        if ((*i)->index == GetBaseLanguageIndex())
        {
          ERR_FATAL(("'%s' does not exist in the base language", path))
        }
      }
    }

    return (vItem);
  }


  //
  // Rename a scope
  //
  VarSys::VarItem *RenameScope(const char *path, const char *name)
  {
    return (RenameKey(path, name));
  }


  //
  // Load
  //
  void Load(const char *language)
  {
    ASSERT(initialized)

    // Load a specific language into the database, 
    // or load all languages into the database
    if (Crc::CalcStr(language) == 0x335DFA3A) // "All"
    {
      // Work out path
      Dir::Find find;
      if (!Dir::FindFirst(find, dbPath.str, "*.cfg"))
      {
        // No languages
        ERR_FATAL(("No languages defined"))
      }

      do
      {
        if (!(find.finddata.attrib & File::SUBDIR))
        {
          LOG_DIAG(("Found language '%s'", find.finddata.name))
          LoadLanguage(NULL, find.finddata.name);
        }
      }
      while (Dir::FindNext(find));
    }
    else
    {
      LoadLanguage(language, NULL);
    }
  }
  

  //
  // Save
  //
  void Save()
  {
    ASSERT(initialized)

    // Save each of the loaded languages
    for (List<Language>::Iterator i(&languages); *i; i++)
    {
      // Build PTree
      PTree pTree;

      // Get the global scope of the parse tree
      FScope *gScope = pTree.GetGlobalScope();

      // Save font information
      char font[sizeof (LOGFONT) * 2 + 1];
      Utils::CharEncode(font, (U8 *) &(*i)->font, sizeof (LOGFONT), sizeof font);
      StdSave::TypeString(gScope, "Font", font);

      // Save database
      FScope *fScope = gScope->AddFunction("DataBase");

      // Work out the name of the database
      StrBuf<260> database;
      database = rootScope;
      Utils::Strcat(database.str, VARSYS_SCOPEDELIMSTR);
      Utils::Strcat(database.str, (*i)->name.str);

      // Find the scope
      VarSys::VarScope *vScope = VarSys::FindVarScope(database.str);
      VarFile::Save(fScope, vScope);
      
      // Generate name of file to write to
      StrBuf<260> file;
      Utils::MakePath(file.str, 260, dbPath.str, (*i)->name.str, ".cfg");

      // Write the file out
      pTree.WriteTreeText(file.str);
    }
  }


  //
  // Build a lookup key
  //
  const char * CDECL BuildKey(U32 params, ...)
  {
    static char buffer[128];

    // The first character in the string is always #
    buffer[0] = '#';
    buffer[1] = '\0';

    // Process the variable arguments
    va_list args;
    va_start(args, params);
    const char *ptr;

    Bool first = TRUE;

    while (params--)
    {
      ptr = va_arg(args, const char *);

      if (first)
      {
        first = FALSE;
      }
      else
      {
        Utils::Strcat(buffer, ".");
      }

      Utils::Strcat(buffer, ptr);
    }
    va_end(args);

    // return the string
    return (buffer);
  }


  //
  // MakePath
  //
  const char * MakePath(const char *path)
  {
    ASSERT(initialized)
    ASSERT(currentLanguage)

    static char buffer[400];
    Utils::Strcpy(buffer, rootScope);
    Utils::Strcat(buffer, VARSYS_SCOPEDELIMSTR);
    Utils::Strcat(buffer, currentLanguage->name.str);

    if (*path)
    {
      Utils::Strcat(buffer, VARSYS_SCOPEDELIMSTR);
      Utils::Strcat(buffer, path);
    }

    return (buffer);
  }


  //
  // MakePath
  //
  const char * MakePath(const char *path, int index)
  {
    ASSERT(initialized)

    // Push the current language
    int oldIndex = GetLanguageIndex();

    // Set the current language
    SetLanguage(index);

    // Get the name
    const char *fullPath = MakePath(path);

    // Pop the current language
    SetLanguage(oldIndex);

    return (fullPath);
  }


  //
  // ComposeName
  //
  static const char *ComposeName(const char *path, const char *name)
  {
    ASSERT(initialized)

    static char buffer[400];

    // Copy the old path into the buffer
    Utils::Strcpy(buffer, path);

    // Find the last '.' in the buffer
    char *ptr = Utils::Strrchr(buffer, VARSYS_SCOPEDELIM);

    if (ptr)
    {
      ptr++;
      Utils::Strcpy(ptr, name);
    }
    else
    {
      // No '.' ... must be root level
      Utils::Strcpy(buffer, name);
    }

    return (buffer);
  }


  //
  // GetItem
  //
  Item * GetItem(const char *path)
  {
    ASSERT(initialized)

    U32 size;
    return (GetItem(path, size));
  }


  //
  // GetItem
  //
  Item * GetItem(const char *path, U32 &size)
  {
    ASSERT(initialized)

    // Make the full path from the current language and the path
    const char *fullPath = MakePath(path);

    // Get pointer to the binary var of this type
    VarSys::VarItem *item = VarSys::FindVarItem(fullPath);

    if (item)
    {
      size = item->BinarySize();
      return ((Item *) item->Binary());
    }
    else
    {
      size = 0;
      return (NULL);
    }
  }


  //
  // Load a language
  //
  void LoadLanguage(const char *name, const char *file)
  {
    ASSERT(initialized)
    ASSERT(name || file)

    StrBuf<260> buffer;

    if (!name)
    {
      // Work out the name from the file
      buffer = file;
      char *ptr = Utils::Strrchr(buffer.str, '.');
      if (*ptr)
      {
        *ptr = '\0';
      }
      name = buffer.str;
    }

    if (!file)
    {
      // Work out the file from the name
      buffer = name;
      Utils::Strcat(buffer.str, ".cfg");
      file = buffer.str;
    }

    // Work out the name of the database
    StrBuf<260> database;
    database = rootScope;
    Utils::Strcat(database.str, VARSYS_SCOPEDELIMSTR);
    Utils::Strcat(database.str, name);

    // If the scope does not exist then create it
    if (!VarSys::FindVarItem(database.str))
    {
      VarSys::VarItem *item = VarSys::CreateVar(rootScope, name);
      item->InitScope();
    }
      
    // Load the user
    PTree pTree;

    // Parse the file
    if (pTree.AddFile(file))
    {
      // Create a new language
      Language *language = new Language;
      language->name = name;
      language->index = languages.GetCount();
      language->font = defaultFont;
      languages.Append(language);

      // Set the current language if it hasn't already been set
      if (!currentLanguage)
      {
        currentLanguage = language;
      }

      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Process each function
      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x78D5CFD4: // "Font"
          {
            const char *font = StdLoad::TypeString(sScope);
            U32 length = Utils::Strlen(font);
            Utils::CharDecode((U8 *)&language->font, font, length, sizeof (LOGFONT));
            break;
          }

          case 0x536B6FC1: // "DataBase"
            VarFile::Load(sScope, database.str);
            break;

          default:
            break;
        }
      }
    }
    else
    {
      if (!Utils::Strncmp(pTree.LastError(), "File has zero length", 20))
      {
        // An empty language
        Language *language = new Language;
        language->name = name;
        language->index = languages.GetCount();
        language->font = defaultFont;
        languages.Append(language);

        // Set the current language if it hasn't already been set
        if (!currentLanguage)
        {
          currentLanguage = language;
        }
      }
      else
      {
        ERR_FATAL(("Could not load Language '%s'", file))
      }
    }
  }


  //
  // Scope Handler
  //
  void Handler(U32 pathCrc)
  {
    ASSERT(initialized)

    pathCrc;
  }

}
