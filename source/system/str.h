///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// String Class
// 1-DEC-1997
//

//
// NOTES
//
// This String Class makes use of a reference counting system to minimize
// unnecesary memory copying.
//
// When assigning a "char *" or "const char *" to a String a pointer is
// taken to that string (a copy is not made).  Care must be taken that this
// memory is not destroyed before the ALL strings refering to it have been
// destroyed. The speed gain for not copying is large and that is why this
// hole has been left in.
//
// StringLtd is identical to String but does not allow the string to become
// larger than the specified size (it will ASSERT).  Usefull for FileNames,
// buts thats about all.
//


#ifndef __STR_H
#define __STR_H


///////////////////////////////////////////////////////////////////////////////
//
// Class String
//
class String
{
protected:

  //
  // STRUCT StringRef
  //
  // Used for reference counting in strings
  //
  struct StringRef
  {
    Bool  owned;
    int   count;
    char  *data;
    U32   size;
    U32   length;

    //
    // StringRef
    //
    // Copy Constructor
    //
    StringRef(const char *c) : count(1), owned(FALSE)
    {
      length = Utils::Strlen(c);
      size = length + 1;
      data = (char *) c;
    }

    //
    // StringRef
    //
    // Reservation Constructor
    //
    StringRef(int r) : count(1), size(r), length(0), owned(TRUE)
    {
      ASSERT(r > 0);
      data = new char[r];
      *data = '\0';
    }

    //
    // ~StringRef
    //
    // Destructor
    //
    ~StringRef()
    {
      if (owned)
      {
        delete [] data;
      }
    }
  };

  StringRef *ref;     // Pointer to String Reference

  //
  // Copy
  //
  void Copy(StringRef *sref)
  {
    ref = sref;
    ref->count++;
  }

  //
  // Resize
  //
  virtual void Resize(U32 size)
  {
    if (ref->size <= size || !ref->owned)
    {
      StringRef *newref = new StringRef(size);
      Utils::Memcpy(newref->data, ref->data, ref->length + 1);
      newref->length = ref->length;
      Dereference();
      ref = newref;
    }
  }

  //
  // Dereference
  //
  void Dereference()
  {
    if (--ref->count == 0)
    {
      delete ref;
    }
  }

public:

  //
  // String
  //
  // Default Constructor
  //
  String() : ref(new StringRef(1)) {}

  //
  // String
  //
  // Initializing Constructor
  //
  // Old Method - String(const char *c) : ref(new StringRef(c)) {}
  //
  String(const char *c)
  {
    U32 len = Utils::Strlen(c);
    StringRef *newref = new StringRef(len + 1);
    Utils::Strcpy(newref->data, c);
    newref->length = len;
    ref = newref;
  }

  //
  // String
  //
  // Copy Constructor
  //
  String(const String &s)
  {
    Copy(s.ref);
  }

  //
  // String
  //
  // Reservation Constructor
  //
  String(U32 size)
  {
    ref = new StringRef(size);
  }

  //
  // ~String
  //
  // Destructor
  //
  ~String()
  {
    Dereference();
  }

  //
  // Dup
  //
  // Forces a duplication
  //
  void Dup(const String s)
  {
    StringRef *newref = new StringRef(s.ref->length + 1);
    Utils::Memcpy(newref->data, s.ref->data, s.ref->length + 1);
    newref->length = s.ref->length;
    Dereference();
    ref = newref;
  }

  //
  // GetLength
  //
  // Returns the length of the string
  //
  U32 GetLength() const
  {
    return (ref->length);
  }

  //
  // Crc
  //
  U32 Crc() const
  {
    return (Crc::CalcLower(ref->data, ref->length));
  }

  //
  // MatchLeft
  //
  // Returns a pointer to the first occurance of 'm' or NULL if not found
  //
  static const char * MatchLeft(const String s, char m)
  {
    return (Utils::Strchr(s, m));
  }
  
  //
  // MatchRight
  //
  // Returns a pointer to the first occurance of 'm' or NULL if not found
  //
  static const char * MatchRight(const String s, char m)
  {
    return (Utils::Strrchr(s, m));
  }

  //
  // operator*
  //
  // Explicit Conversion (when using String as paramater for vsprintf)
  //
  const char * operator*() const
  {
    return (ref->data);
  }

  //
  // operator const char *
  //
  // Implicit Conversion
  //
  operator const char * () const
  {
    return (ref->data);
  }

  //
  // operator[]
  //
  // Retrive a particular character
  //
  char operator[](int index) const
  {
    return (ref->data[index]);
  }

  //
  // operator=
  //
  String & operator=(const String &s)
  {
    // Make sure that we're not assigning to ourself
    if (ref == s.ref)
    {
      return (*this);
    }

    // Dereference the reference
    Dereference();

    // Copy the reference from the other string
    Copy(s.ref);

    return (*this);
  }

  //
  // operator+=
  //
  String & operator+=(const String &s)
  {
    Resize(ref->length + s.ref->length + 1);
    Utils::Memcpy(ref->data + ref->length, s.ref->data, s.ref->length + 1);
    ref->length += s.ref->length;

    return (*this);    
  }

  //
  // operator+
  //
  String operator+(const String &s) const
  {
    String tmp(*this);
    tmp += s;
    return (tmp);
  }

  //
  // operator==
  //
  // Tests to see if two strings are equivalent
  //
  int operator==(const String &s) const
  {
    return (!Utils::Strcmp(ref->data, s.ref->data));
  }

  //
  // operator!=
  //
  int operator!=(const String &s) const
  {
    return (Utils::Strcmp(ref->data, s.ref->data));
  }

  //
  // operator<
  //
  int operator<(const String &s) const
  {
    return (Utils::Strcmp(ref->data, s.ref->data) < 0);
  }

  //
  // operator>
  //
  int operator>(const String &s) const
  {
    return (Utils::Strcmp(ref->data, s.ref->data) > 0);
  }

  //
  // operator<=
  //
  int operator<=(const String &s) const
  {
    return (Utils::Strcmp(ref->data, s.ref->data) <= 0);
  }

  //
  // operator>=
  //
  int operator>=(const String &s) const
  {
    return (Utils::Strcmp(ref->data, s.ref->data) >= 0);
  }

  //
  // operator==
  //
  int operator==(const char *c) const
  {
    return (!Utils::Strcmp(ref->data, c));
  }

  //
  // operator!=
  //
  int operator!=(const char *c) const
  {
    return (Utils::Strcmp(ref->data, c));
  }

  //
  // ItoA
  //
  static String ItoA(int value, int radix)
  {
    String str;
    char buff[32];
    Utils::ItoA(value, buff, radix);
    str.Dup(buff);
    return (str);
  }

  //
  // Fill
  //
  // Fills string with 'n' of 'c'
  //
  void Fill(U32 n, char c)
  {
    Resize(n + 1);
    Utils::Memset(ref->data, c, n);
    ref->data[n] = '\0';
    ref->length = n;
  }

  //
  // Make
  //
  static String CDECL Make(const char *format, ...);

};



///////////////////////////////////////////////////////////////////////////////
//
// Template StringLtd
//
template <U32 SIZE> class StringLtd : public String
{
protected:

  void Resize(U32 size)
  {
    ASSERT(size < SIZE)
    String::Resize(size);
  }

public:

  //
  // StringLtd<SIZE>
  //
  // Default constructor
  //
  StringLtd<SIZE>() : String() {}

  //
  // StringLtd<SIZE>
  //
  // Assignment Constructor
  //
  StringLtd<SIZE>(const char *c) : String(c) {}

  //
  // StringLtd<SIZE>
  //
  // Copy constructor which takes String
  //
  StringLtd<SIZE>(const String &s) : String(s) {}

};

#endif


