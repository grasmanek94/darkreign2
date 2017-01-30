///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Registry Interface
// 26-NOV-1997
//


//
// NOTES
//
// The registry is extremely Win32 specific
//
// The predefined keys are
//
// HKEY_CLASSES_ROOT
// HKEY_CURRENT_USER
// HKEY_LOCAL_MACHINE
// HKEY_USERS
//


#ifndef __REGISTRY_H
#define __REGISTRY_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// Class Registry
//
class Registry
{
public:

  //
  // Access mods
  //
  enum Access
  {
    ALL             = KEY_ALL_ACCESS,
    CREATE_LINK     = KEY_CREATE_LINK,
    CREATE_SUB_KEY  = KEY_CREATE_SUB_KEY,
    ENUM            = KEY_ENUMERATE_SUB_KEYS,
    EXECUTE         = KEY_EXECUTE,
    NOTIFY          = KEY_NOTIFY,
    QUERY           = KEY_QUERY_VALUE,
    READ            = KEY_READ,
    SET             = KEY_SET_VALUE,
    WRITE           = KEY_WRITE
  };

  //
  // Data Types
  //
  enum Type
  {
    BINARY          = REG_BINARY,         // Binary data in any form. 
    INTEGER         = REG_DWORD,          // 32-bit number. 
    EXPAND_STRING   = REG_EXPAND_SZ,      // A null-terminated string that contains unexpanded references to environment variables (for example, %PATH%). It will be a Unicode or ANSI string depending on whether you use the Unicode or ANSI functions. 
    UNICODE_LINK    = REG_LINK,           // A Unicode symbolic link. 
    STRINGS         = REG_MULTI_SZ,       // An array of null-terminated strings, terminated by two null characters. 
    NONE            = REG_NONE ,          // No defined value type. 
    RESOURCE_LIST   = REG_RESOURCE_LIST,  // A device-driver resource list. 
    STRING          = REG_SZ,             // A null-terminated string. It will be a Unicode or ANSI string depending on whether you use the Unicode or ANSI functions. 
  };

  //
  // Delete
  //
  // Delete a key
  //
  static Bool Delete(HKEY key, String name)
  {
    return ((RegDeleteKey(key, name) == ERROR_SUCCESS) ? TRUE : FALSE);
  }

  class Key
  {
  private:

    HKEY  key;
    U32   disp;
    Bool  notFound;

  public:

    //
    // Key
    //
    // Construction using a known key handle
    //
    Key(HKEY base, String name, Access access = ALL, Bool create = TRUE)
    {
      notFound = FALSE;

      if (create)
      {
        if (RegCreateKeyEx(base, name, NULL, "", REG_OPTION_NON_VOLATILE, access, NULL, &key, &disp) != ERROR_SUCCESS)
        {
          MessageBox(NULL, Debug::LastError(), "RegCreateKeyEx error", MB_OK | MB_ICONSTOP);
        }
      }
      else
      {
        switch (RegOpenKeyEx(base, name, NULL, access, &key))
        {
          case ERROR_SUCCESS:
            disp = 0;
            break;
        
          case ERROR_FILE_NOT_FOUND:
            disp = REG_CREATED_NEW_KEY;
            break;

          default:
            disp = REG_CREATED_NEW_KEY;
            notFound = TRUE;
            break;
        }
      }
    }
 

    //
    // Key
    //
    // Construction using another key class
    //
    Key(const Key &base, String name, Access access = ALL)
    {
      if (RegCreateKeyEx(base.key, name, NULL, "", REG_OPTION_NON_VOLATILE, access, NULL, &key, &disp) != ERROR_SUCCESS)
      {
        MessageBox(NULL, Debug::LastError(), "RegOpenKeyEx error", MB_OK | MB_ICONSTOP);
      }
    }


    //
    // ~Key
    //
    ~Key()
    {
      RegCloseKey(key);
    }


    //
    // operator HKEY
    //
    operator HKEY()
    {
      return (key);
    }


    //
    // NotFound
    //
    Bool NotFound()
    {
      return (notFound);
    }


    //
    // IsNew
    //
    Bool IsNew()
    {
      return (disp == REG_CREATED_NEW_KEY);
    }


    //
    // GetValue
    //
    // Retrieve a value from a key 
    //
    Bool GetValue(String name, Type *type, U8 *data, U32 *length)
    {
      if (RegQueryValueEx(key, name, NULL, (U32 *) type, data, length) != ERROR_SUCCESS)
      {
      //  MessageBox(NULL, Debug::LastError(), "RegQueryValueEx error", MB_OK | MB_ICONSTOP);
        return (FALSE);
      }
      else
      {
        return (TRUE);
      }
    }


    //
    // SetValue
    //
    // Set the value of a key
    //
    Bool SetValue(String name, Type type, const U8 *data, U32 length)
    {
      if (RegSetValueEx(key, name, NULL, type, data, length) != ERROR_SUCCESS)
      {
      //  MessageBox(NULL, Debug::LastError(), "RegSetValueEx error", MB_OK | MB_ICONSTOP);
        return (FALSE);
      }
      else
      {
        return (TRUE);
      }

    }


    //
    // GetInt
    //
    // Get Int for the name specified
    // Returns 0 if could not retrieve value
    //
    int GetInt(String name)
    {
      Type  type;
      int   data;
      U32   length = sizeof (U32);

      if (GetValue(name, &type, (U8 *) &data, &length) && (type == INTEGER))
      {
        return (data);
      }
      else
      {
        return (0);
      }
    }


    //
    // SetInt
    //
    // Sets Int for the name specified
    // Returns TRUE if successful or false if failed
    //
    Bool SetInt(String name, int data)
    {
      return (SetValue(name, INTEGER, (U8 *) &data, sizeof (int)));
    }


    //
    // GetString
    //
    // Can only retrieve strings up to 1024 characters in length
    // If it cannot retrieve the string it returns a NULL string
    //
    char * GetString(const char *name, char *string, U32 *length)
    {
      Type type;

      if (GetValue(name, &type, (U8 *) string, length) && (type == STRING))
      {
        return (string);
      }
      else
      {
        return (NULL);
      }
    }


    //
    // SetString
    //
    // Sets a string in the registry
    //
    Bool SetString(const char *name, char *string, U32 length)
    {
      return (SetValue(name, STRING, (U8 *) string, length));
    }


    //
    // GetData
    //
    // Gets some RAW binary data from the registry
    // Returns TRUE if successful and FALSE if it failed
    // This function will corrupt the contents of data even if it fails
    //
    Bool GetData(String name, U8 *data, U32 length)
    {
      Type type;
      U32  len = length;
      return (GetValue(name, &type, data, &len) && (type == BINARY));
    }


    //
    // SetData
    //
    // Sets some RAW binary data in the registry
    // Returns TRUE if successful and FALSE if it failed
    //
    Bool SetData(String name, const U8 *data, U32 length)
    {
      return (SetValue(name, BINARY, data, length));
    }


    //
    // Enum
    //
    // Enumerates the subkeys of the current key
    // Returns TRUE if the index was found
    // Returns FALSE if the index was out of range
    //
    Bool Enum(U32 index, String &s)
    {
      char  buff[256];
      U32   length = 256;

      switch (RegEnumKeyEx(key, index, buff, &length, NULL, NULL, NULL, NULL))
      {
        case ERROR_SUCCESS:
          s.Dup(buff);
          return (TRUE);
          break;

        case ERROR_NO_MORE_ITEMS:
          return (FALSE);
          break;

        default:
          MessageBox(NULL, Debug::LastError(), "RegEnumKeyEx error", MB_OK | MB_ICONSTOP);
          return (FALSE);
          break;
      }
    }


    //
    // EnumValue
    //
    // Enumerates the subvalues of the current key
    // Returns TRUE if the index was found
    // Returns FALSE if the index was out of range
    //
    Bool EnumValue(U32 index, String &s)
    {
      char  buff[256];
      U32   length = 256;

      switch (RegEnumValue(key, index, buff, &length, NULL, NULL, NULL, NULL))
      {
        case ERROR_SUCCESS:
          s.Dup(buff);
          return (TRUE);
          break;

        case ERROR_NO_MORE_ITEMS:
          return (FALSE);
          break;

        default:
          MessageBox(NULL, Debug::LastError(), "RegEnumKeyEx error", MB_OK | MB_ICONSTOP);
          return (FALSE);
          break;
      }
    }


    //
    // Delete
    //
    // Delete a subkey
    //
    Bool Delete(String name)
    {
      return ((RegDeleteKey(key, name) == ERROR_SUCCESS) ? TRUE : FALSE);
    }
  };
};

#endif