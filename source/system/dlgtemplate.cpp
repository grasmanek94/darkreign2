///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dialog Template Code
//
// 14-DEC-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "dlgtemplate.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class DlgTemplate
//


//
// Constructor
//
DlgTemplate::DlgTemplate(const char *title, U16 x, U16 y, U16 width, U16 height, U32 style)
{
  ASSERT(title)

  // Clear the buffer
  memset(buffer, 0x00, sizeof (buffer));

  // Set pointer to start of buffer
  ptr = buffer;

  // ExStyle is ignored so set it to zero
  U32 exStyle = 0;

  // The first thing to do is to fill out the template header
  *ptr++ = LOWORD(style);     // LOWORD (style)
  *ptr++ = HIWORD(style);     // HIWORD (style)
  *ptr++ = LOWORD(exStyle);   // LOWORD (exStyle)
  *ptr++ = HIWORD(exStyle);   // HIWORD (exStyle)

  // Save pointer to number of items
  numItems = ptr;
  *ptr++ = 0;                 // NumberOfItems
  *ptr++ = x;                 // x
  *ptr++ = y;                 // y
  *ptr++ = width;             // cx
  *ptr++ = height;            // cy
  *ptr++ = 0;                 // Menu
  *ptr++ = 0;                 // Class
  ptr += Utils::Ansi2Unicode(ptr, DLGTEMPLATE_BUFFSIZE + ptr - buffer, title);
  *ptr++ = 8;                 // Point Size
  ptr += Utils::Ansi2Unicode(ptr, DLGTEMPLATE_BUFFSIZE + ptr - buffer, "MS Sans Serif");

  AlignPtr();
}


//
// Destructor
//
DlgTemplate::~DlgTemplate()
{
}


//
// AlignPtr
//
DlgTemplate::AlignPtr()
{
  // Move pointer to a U32 boundary
  ptr = (U16 *) (((((U32) ptr) + 3) >> 2) << 2);
}


//
// AddItem
//
void DlgTemplate::AddItem(U16 x, U16 y, U16 width, U16 height, U16 id, U32 style, const char *type, const char *text, U32 exStyle)
{
  // Increment number of items
  (*numItems)++;

  // Add the item to the template
  *ptr++ = LOWORD(style);     // LOWORD (style)
  *ptr++ = HIWORD(style);     // HIWORD (style)
  *ptr++ = LOWORD(exStyle);   // LOWORD (exStyle)
  *ptr++ = HIWORD(exStyle);   // HIWORD (exStyle)
  *ptr++ = x;                 // x
  *ptr++ = y;                 // y
  *ptr++ = width;             // cx
  *ptr++ = height;            // cy
  *ptr++ = id;                // identification
  ptr += Utils::Ansi2Unicode(ptr, DLGTEMPLATE_BUFFSIZE + ptr - buffer, type);
  ptr += Utils::Ansi2Unicode(ptr, DLGTEMPLATE_BUFFSIZE + ptr - buffer, text);
  *ptr++ = 0;                 // Advance pointer over nExtraStuff WORD

  AlignPtr();
}


//
// AddItem
//
void DlgTemplate::AddItem(U16 x, U16 y, U16 width, U16 height, U16 id, U32 style, U16 typeClass, const char *text, U32 exStyle)
{
  // Force some settings
  style |= DS_SETFONT | WS_VISIBLE | WS_CHILD;

  // Increment number of items
  (*numItems)++;

  // Add the item to the template
  *ptr++ = LOWORD(style);     // LOWORD (style)
  *ptr++ = HIWORD(style);     // HIWORD (style)
  *ptr++ = LOWORD(exStyle);   // LOWORD (exStyle)
  *ptr++ = HIWORD(exStyle);   // HIWORD (exStyle)
  *ptr++ = x;                 // x
  *ptr++ = y;                 // y
  *ptr++ = width;             // cx
  *ptr++ = height;            // cy
  *ptr++ = id;                // identification
  *ptr++ = (U16) 0xFFFF;
  *ptr++ = typeClass;         // class
  ptr += Utils::Ansi2Unicode(ptr, DLGTEMPLATE_BUFFSIZE + ptr - buffer, text);
  *ptr++ = 0;                 // Advance pointer over nExtraStuff WORD

  AlignPtr();
}
