///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dynamic game variable management system
//
// 10-APR-1998
//

#ifndef __VARSYS_H
#define __VARSYS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "tracksys.h"
#include "str.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Max size of any full var path
#define VARSYS_MAXVARPATH   256

// Delimiter between scopes and vars (single char only)
#define VARSYS_SCOPEDELIM     '.'
#define VARSYS_SCOPEDELIMSTR  "."

// Standard command call-back
typedef void (VarSysCallBack)(U32);

// Var Path Identifier - Buffer that holds full item path
typedef StrCrc<VARSYS_MAXVARPATH> VarPathIdent;

// Forward references
class VarString;
class VarInteger;
class VarFloat;
class VarBinary;

struct VarNotify;


///////////////////////////////////////////////////////////////////////////////
//
// Class VarSys - Manages all var scopes
//

class VarSys
{
public:

  // Property flags
  enum
  {
    DEFAULT   = 0x0000, 
    NOTIFY    = 0x0001,
    NOEDIT    = 0x0004,
    CLAMP     = 0x0008,
    RAWVAR    = 0x0010,
    RAWDATA   = 0x0020,
  };

  // For forward reference
  struct VarScope;

  // Struct Substitution
  struct Substitution
  {
    // Character for substitution to occur on
    char ch;

    // Tree node
    NBinTree<Substitution, char>::Node node;

    // Constructor
    Substitution(char ch)
    : ch(ch)
    {
    }

    // Expansion of var names
    virtual const char *Expand(const char *name, void *context) = 0;
  };

  // Possible var item types
  enum VarItemType
  {
    // Not yet setup
    VI_NONE,

    // String value
    VI_STRING,

    // Integer value
    VI_INTEGER,

    // Floating point value
    VI_FPOINT,

    // Binary value
    VI_BINARY,

    // A command
    VI_CMD,

    // A sub-scope
    VI_SCOPE,

    // For assertions
    VI_MAXIMUM
  };

  // String representations
  static char* itemTypeStrings[VI_MAXIMUM];

  // A single var item
  struct VarItem
  {
    // Death tracking info
    DTrack::Info dTrack;

    // List of objects to be updated when the value changes
    VarNotify *pNotify;

    // All items node
    NBinTree<VarItem>::Node allItemsNode;

    // Data for each type
    union
    {
      // STRING type
      struct 
      {
        String *val;
      } string;

      // INTEGER type
      struct 
      {
        S32 val;
        S32 lo;
        S32 hi;
      } integer;

      // FPOINT type
      struct 
      {
        F32 val;
        F32 lo;
        F32 hi;
      } fpoint;

      // BINARY type
      struct
      {
        U32 maxSize;
        U32 size;
        U8  *data;
      } binary;

      // SCOPE type
      struct 
      {
        VarScope *ptr;
      } scope;
    };

    // Trigger parent's call back function, if any
    void TriggerCallBack();

    // Initialize as STRING type
    void InitString(const char *val);

    // Initialize as INTEGER type
    void InitInteger(S32 val);

    // Initialize as FPOINT type
    void InitFPoint(F32 val);

    // Initialize as BINARY value
    void InitBinary(U32 size, const U8 *val = NULL);

    // Initialize as CMD type
    void InitCmd();

    // Initialize as SCOPE type (creates a new sub-scope)
    void InitScope();

    // SCOPE type - returns pointer to sub-scope
    VarScope* ScopePtr();

    // Clear this item (ie. so can be re-initialized)
    void Clear();

    // Item property flags
    U32 flags;

    // Type of this scope item
    VarItemType type;

    // Unique item ident
    GameIdent itemId;

    // Crc of full item path, passed to command functions
    U32 pathCrc;

    // Parent scope (always valid)
    VarScope *pScope;


    // Constructor
    VarItem(const char *identIn, U32 pathCrcIn, VarScope *parent);

    // Destructor
    ~VarItem();

    // Get the string value of a var
    const char * GetStringValue();

    // Get the unicode string value of a var
    const CH * GetUnicodeStringValue();

    // STRING type - standard access methods
    String Str();
    void SetStr(const char *newVal);

    // INTEGER type - standard access methods
    S32 Integer();
    void SetInteger(S32 newVal);
    void SetIntegerRange(S32 low, S32 high);

    // FPOINT type - standard access methods
    F32 Float();
    void SetFloat(F32 newVal);
    void SetFloatRange(F32 low, F32 high);

    // BINARY type - standard access methods
    const U8 *Binary();
    U32 BinarySize();
    void SetBinary(U32 size, const U8 *newVal = NULL);

    // CMD type - trigger call back
    void TriggerCmd();
    void DispatchCmd();

    // Add an object to the list of objects to be notified upon change of value
    void NotifyMe(VarNotify *obj);

    // Opposite of NotifyMe
    void ForgetMe(VarNotify *obj);

    // Set some of the flags
    void SetFlags(U32 flagMask);

    // Clear some of the flags
    void ClearFlags(U32 flagMask);

  };

  // A single var scope, containing a list of var items
  struct VarScope
  {
    // Creates a new item within this scope, setting the parent
    VarItem* CreateNewItem(const char *identIn, U32 pathCrcIn);

    // Delete an item from this scope
    void DeleteItem(VarItem *item);

    // Scope flags
    U32 flags;
    
    // Tree of items in this scope
    BinTree<VarItem> items;

    // Scope command handler
    VarSysCallBack *callBack;

    // Constructor & destructor
    VarScope();
    ~VarScope();

    // Returns item with ident matching 'crc', or NULL if not found
    VarItem* FindItem(U32 crc);
  };

  // System initialized flag
  static Bool sysInit;

  // Pointer to the global var scope
  static VarScope *gScope;

  // Tree of all existing var items
  static NBinTree<VarItem> allItems;

  // Tree of all substitutions
  static NBinTree<Substitution, char> substitutions;
    
  // Splits the delimited path 'source' into two sections
  static Bool SplitDelimited(String &one, String &two, String source, Bool matchLeft);

  // Creates a new var item (returned item is NOT setup)
  static VarItem* CreateVarItem(const char *path, void *context = NULL);

  // Returns a pointer to the var item 'pathCrc', or NULL if not found
  static VarItem* FindVarItemHelper(U32 pathCrc, const char *name);
  
public:

  // Intialise the var system
  static void Init();

  // Shutdown var system
  static void Done();

  // Dynamic Substitudion
  static void RegisterSubstitution(Substitution &substiution);
  static void UnregisterSubstitution(Substitution &substiution);
  
  // Build a var name performing substitution
  static const char *BuildVarName(const char *path, void *context = NULL);

  // Find a var using substitution, or NULL if not found
  static VarItem* FindVarItem(const char *path, void *context = NULL, Bool required = FALSE);
  
  // Find a var using a path and an identifier
  static VarItem* FindVarItem(const char *path, const char *ident, void *context = NULL);

  // Returns TRUE if the item 'path' exists, AND is a VARIABLE (ie. not cmd)
  static Bool FindVariable(const char *path);

  // Returns a pointer to the var scope 'path', or NULL if not found
  static VarScope* FindVarScope(const char *path);

  // Copy a var (src MUST exist)
  static VarItem* CopyVarItem(const char *dst, const char *src);

  // Copies the var item in src to a new var item called dst
  static VarItem* CopyVarItem(const char *dst, VarItem *src);

  // Registers a command handler for scope 'path', creating if not found
  static void RegisterHandler(const char *path, VarSysCallBack *func, U32 flagsIn = DEFAULT);

  // Registers a command handler for the root scope
  static void RegisterRootHandler(VarSysCallBack *func);

  // Create an empty Var (requires that path is a scope!)
  static VarItem* CreateVar(const char *path, const char *ident, void *context = NULL);

  // Create a SCOPE using the given path (ignored if already exists)
  static VarItem* CreateScope(const char *path, U32 flagsIn = DEFAULT);

  // Create a STRING variable
  static VarItem* CreateString(const char *path, const char *value, U32 flagsIn = DEFAULT, VarString *var = NULL, void *context = NULL);

  // Create an INTEGER variable
  static VarItem* CreateInteger(const char *path, S32 value, U32 flagsIn = DEFAULT, VarInteger *var = NULL, void *context = NULL);

  // Create a FPOINT variable
  static VarItem* CreateFloat(const char *path, F32 value, U32 flagsIn = DEFAULT, VarFloat *var = NULL, void *context = NULL);

  // Create a BINARY variable
  static VarItem* CreateBinary(const char *path, U32 size, const U8 *value = NULL, U32 flagsIn = DEFAULT, VarBinary *var = NULL, void *context = NULL);

  // Create a CMD item
  static void CreateCmd(const char *path, U32 flagsIn = DEFAULT, void *context = NULL);

  // Delete an item
  static void DeleteItem(VarItem *item);

  // Delete an item, returning FALSE if item was not found
  static Bool DeleteItem(const char *path, void *context = NULL);

  // Set the range of an integer
  static Bool SetIntegerRange(const char *path, S32 low, S32 high);

  // Set the range of a float
  static Bool SetFloatRange(const char *path, F32 low, F32 high);

  // Trigger the command item pointed to by 'path', FALSE if not found
  static Bool TriggerCmd(const char *path);

  // returns a string representation of 'type'
  static char* GetTypeString(VarItemType type);
};



///////////////////////////////////////////////////////////////////////////////
//
// struct VarNotify
//
// Abstract base class for an object that needs to be notified when the 
// value of a var changes.
//

struct VarNotify
{
  enum Mode
  {
    CHANGED,
    DELETED,
  };

  // Next item in the notify list
  VarNotify *next;

  // Constructor
  VarNotify()
  {
    next = NULL;
  }

  // Gets called on var notify event
  virtual void Notify(Mode mode) = 0;
};



///////////////////////////////////////////////////////////////////////////////
//
// Class VarPtr - Abstract base class for a pointer to a var item
//

class VarPtr
{
protected:

  // Validates pointer, and generates error if not valid (debug)
  void Validate()
  {
    if (!item.Alive())
    {
      ERR_FATAL(("Attempt to use an invalid VarPtr (not setup or target has been deleted)"))
    }
  }

public:

  // Death tracker target
  Reaper<VarSys::VarItem> item;


public:

  // Returns TRUE if this VarPtr is valid
  Bool Alive()
  {
    return (item.Alive());
  }

  // Delete the var item this is pointing to
  void Delete()
  {
    if (item)
    {
      VarSys::DeleteItem(item);
    }
  }

  // Access to VarItem
  VarSys::VarItem* operator->()
  {
    return (item);
  }

  // Add an object to the list of objects to be notified upon change of value
  void NotifyMe(VarNotify *obj)
  {
    (*item).NotifyMe(obj);
  }

  // Opposite of NotifyMe
  void ForgetMe(VarNotify *obj)
  {
    (*item).ForgetMe(obj);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class VarString - Points to a String game variable
//

class VarString : public VarPtr
{
public:

  // Constructor
  VarString() {};

  // Constructor
  VarString(const char *path, void *context = NULL);

  // Constructor
  VarString(VarSys::VarItem *var);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(const char *path, void *context = NULL);

  // Setup/change to point to the var
  void PointAt(VarSys::VarItem *var);

  // Setup/change to point to the VarPtr 'var', which MUST be setup
  void PointAt(VarString &var);

  // Operators
  VarString& operator=(const char *val);
  VarString& operator=(VarString &val);
  operator const char *();
  const char * operator*();
};


///////////////////////////////////////////////////////////////////////////////
//
// Class VarInteger - Points to an S32 game variable
//

class VarInteger : public VarPtr
{
public:

  // Constructor
  VarInteger() {};

  // Constructor
  VarInteger(const char *path, void *context = NULL);

  // Constructor
  VarInteger(VarSys::VarItem *var);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(const char * path, void *context = NULL);

  // Seutp/change to point to the var
  void PointAt(VarSys::VarItem *var);

  // Setup/change to point to the VarPtr 'var', which MUST be setup
  void PointAt(VarInteger &var);

  // Operators
  VarInteger& operator=(S32 val);
  VarInteger& operator=(VarInteger &val);
  operator S32 ();
  S32 operator*();
};


///////////////////////////////////////////////////////////////////////////////
//
// Class VarFloat - Points to an F32 game variable
//

class VarFloat : public VarPtr
{
public:

  // Constructor
  VarFloat() {};

  // Constructor
  VarFloat(const char *path, void *context = NULL);

  // Constructor
  VarFloat(VarSys::VarItem *var);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(const char *path, void *context = NULL);

  // Setup/change to point to the var
  void PointAt(VarSys::VarItem *var);

  // Setup/change to point to the VarPtr 'var', which MUST be setup
  void PointAt(VarFloat &var);

  // Operators
  VarFloat& operator=(F32 val);
  VarFloat& operator=(VarFloat &val);
  operator F32 ();
  F32 operator*();
};


///////////////////////////////////////////////////////////////////////////////
//
// Class VarBinary - Points to a BINARY game variable
//

class VarBinary : public VarPtr
{
public:

  // Constructor
  VarBinary() {};

  // Constructor
  VarBinary(const char *path, void *context = NULL);

  // Constructor
  VarBinary(VarSys::VarItem *var);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(const char *path, void *context = NULL);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(VarSys::VarItem *var);

  // Setup/change to point to the VarPtr 'var', which MUST be setup
  void PointAt(VarBinary &var);

  // Set
  void Set(U32 size, const U8 *val);

  // Operators
  // Note: Use with care, assumes that the pointer points to the same size as the Binary var's data
  VarBinary& operator=(const U8 *val);  
  VarBinary& operator=(VarBinary &val);
  operator const U8 *();
  const U8 * operator*();

  // Obtain the size of the binary var
  U32 GetSize();
};


///////////////////////////////////////////////////////////////////////////////
//
// Class VarCmd - Points to a command item
//

class VarCmd : public VarPtr
{
public:

  // Constructor
  VarCmd() {};

  // Constructor
  VarCmd(const char *path, void *context = NULL);

  // Constructor
  VarCmd(VarSys::VarItem *var);

  // Setup/change to point to the var 'path', which MUST exist
  void PointAt(const char *path, void *context = NULL);

  // Setup/change to point to the var
  void PointAt(VarSys::VarItem *var);

  // Setup/change to point to the VarPtr 'var', which MUST be setup
  void PointAt(VarCmd &var);

  // Operators
  void Trigger();
};

#endif
