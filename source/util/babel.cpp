///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "babel.h"
#include "multilanguage.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define BABEL_NUMBUFFERS 5
#define BABEL_MAXFORMAT 512 // FIXME: this is rather large


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Babel
//
namespace Babel
{
  LOGDEFLOCAL("Babel")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Sub
  //
  struct Sub
  {
    CH type;
    CH format[BABEL_MAXFORMAT];
  };


  // Buffers for Unicode translation
  static CH buffers[BABEL_NUMBUFFERS][MULTILANGUAGE_MAXDATA];
  static int bufferIndex = 0;
  static const CH *error = L"Error";


  //
  // Translate
  //
  const CH * CDECL Translate(const char *key, U32 params, ...)
  {
    va_list args;
    va_start(args, params);
    const CH *ptr = Translate(args, params, key);
    va_end(args);
    return (ptr);
  }


  //
  // Translate
  //
  const CH * Translate(va_list args, U32 params, const char *key)
  {
    CH buffer[MULTILANGUAGE_MAXDATA];

    // Do we need to rotate the buffers
    if (bufferIndex == BABEL_NUMBUFFERS)
    {
      bufferIndex = 0;
    }

    // If the first character # ?
    if (*key == '#')
    {
      // Use the multilanguage system to lookup the key

      // Advance key over delimeter
      key++;

      // The rest of the string is assumed to be a multilanguage key
      CH *ptr;
      U32 size;
      MultiLanguage::Lookup(key, ptr, size);

      if (!ptr)
      {
        static CH unknown[128];

        #ifdef DEVELOPMENT
          Utils::Sprintf(unknown, 128, L"Unknown[%s]", Utils::Ansi2Unicode(key));
        #else
          Utils::Sprintf(unknown, 128, L"%s", Utils::Ansi2Unicode(key));
        #endif

        //LOG_ERR(("Multilanguage Key '%s' not found [%s]", key, MultiLanguage::GetLanguage()))
        ptr = unknown;
        size = Utils::Strlen(unknown) * sizeof (CH);
      }

      // Copy into the buffer
      Utils::Memcpy(buffer, ptr, size);

      // Null terminate
      buffer[size / sizeof (CH)] = CH('\0');
    }
    else
    {
      // Convert the given text into unicode
      #if 0

        if (*key)
        {
          LOG_WARN(("Non keyed string '%s'", key))
        }

      #endif

      Utils::Ansi2Unicode(buffer, MULTILANGUAGE_MAXDATA, key);
    }

    // Sprintf into the next available slot
    Sprintf(buffers[bufferIndex], MULTILANGUAGE_MAXDATA, args, params, buffer);

    // Return the buffer
    return (buffers[bufferIndex++]);
  }


  //
  // Sprintf
  //
  void CDECL Sprintf(CH *buffer, U32 size, const CH *format, U32 params, ...)
  {
    va_list args;
    va_start(args, params);
    Sprintf(buffer, size, args, params, format);
    va_end(args);
  }


  //
  // Sprintf
  //
  void Sprintf(CH *dst, U32 size, va_list args, U32 params, const CH *format)
  {
    size; // Yeah, we'll do something with this someday

    // Look through the text for '{' which may indicate a substitution should take place
    BinTree<Sub> subs;
    CH *ptr = Utils::Strchr(format, CH('{'));

    while (ptr)
    {
      // If the next character is also '{' ignore
      if (*(ptr + 1) != CH('{'))
      {
        ptr++;
        
        // Retrieve the index
        U32 index = Utils::AtoI(ptr);

        // Make sure its in range
        if (index > params)
        {
          // Report error
          LOG_ERR(("Error parsing '%s' : Index %d is not supported", Utils::Unicode2Ansi(format), index))

          // Clean up our stuff
          subs.DisposeAll();

          // Return error
          Utils::Strcpy(dst, error);
          return;
        }

        // Find the ':'
        ptr = Utils::Strchr(ptr, CH(':'));

        if (!ptr)
        {
          // Report error
          LOG_ERR(("Error parsing '%s' : Expected ':'", Utils::Unicode2Ansi(format)))

          // Clean up our stuff
          subs.DisposeAll();

          // Return error
          Utils::Strcpy(dst, error);
          return;
        }

        // The text from the ':' to the '}' is the text we need for formatting
        Sub *sub = new Sub;
        CH *format = sub->format;
        subs.Add(index, sub);

        // First character of formating should be '%'
        (*format++) = CH('%');

        ptr++;
        while (*ptr != CH('}'))
        {
          if (!*ptr)
          {
            // Report error
            LOG_ERR(("Error parsing '%s' : Expected '}'", Utils::Unicode2Ansi(format)))

            // Clean up our stuff
            subs.DisposeAll();

            // Return error
            Utils::Strcpy(dst, error);
            return;
          }

          *(format++) = *(ptr++);
        }

        // Save the last character as the type
        sub->type = *(format - 1);

        // Terminate the format
        *(format) = CH('\0');
      }
      ptr = Utils::Strchr(ptr, CH('{'));
    }

    // Use arguments passed in to fill out the substitutions
    U32 index = 1;
    for (BinTree<Sub>::Iterator i(&subs); *i; i++)
    {
      if (i.GetKey() != index)
      {
        // Report error
        LOG_ERR(("Error parsing '%s' : Index %d is missing!", Utils::Unicode2Ansi(format), index))

        // Clean up our stuff
        subs.DisposeAll();

        // Return error
        Utils::Strcpy(dst, error);
        return;
      }

      // Increment index
      index++;

      CH temp[BABEL_MAXFORMAT];

      switch ((*i)->type)
      {
        case CH('c'):
        case CH('C'):
        case CH('d'):
        case CH('i'):
        case CH('o'):
        case CH('u'):
        case CH('x'):
        case CH('X'):
          Utils::Sprintf(temp, BABEL_MAXFORMAT, (*i)->format, va_arg(args, int));
          break;

        case CH('e'):
        case CH('E'):
        case CH('f'):
        case CH('g'):
        case CH('G'):
          Utils::Sprintf(temp, BABEL_MAXFORMAT, (*i)->format, va_arg(args, double));
          break;

        case CH('n'):
          Utils::Sprintf(temp, BABEL_MAXFORMAT, (*i)->format, va_arg(args, int *));
          break;

        case CH('p'):
          Utils::Sprintf(temp, BABEL_MAXFORMAT, (*i)->format, va_arg(args, void *));
          break;

        case CH('s'):
        case CH('S'):
          Utils::Sprintf(temp, BABEL_MAXFORMAT, (*i)->format, va_arg(args, const CH *));
          break;
      }

      Utils::Strcpy((*i)->format, temp);
    }

    // Now copy the format into one of the slots and substitute the text over
    const CH *src = format;
    ptr = Utils::Strchr(format, CH('{'));
    while (ptr)
    {
      // If the next character is also '{' ignore
      if (*(ptr + 1) != CH('{'))
      {
        // Copy over up to this point
        U32 len = (ptr - src);
        if (len)
        {
          Utils::Strncpy(dst, src, len);
          src += len;
          dst += len;
        }
        ptr++;

        // Retrieve the index
        int index = Utils::AtoI(ptr);

        // Retrieve formatting
        Sub *sub = subs.Find(index);
        ASSERT(sub)

        // Copy sub into dest
        Utils::Strcpy(dst, sub->format);

        // Advance the dest
        dst += Utils::Strlen(sub->format);

        // Advance the src to the corresponding '}'
        ptr = Utils::Strchr(ptr, '}');
        ASSERT(ptr)

        ptr++;
        src = ptr;
      }
      ptr = Utils::Strchr(ptr, CH('{'));
    }

    // Cleanup
    subs.DisposeAll();

    // Copy what evers left over
    Utils::Strcpy(dst, src);
  }

}
