///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dialog Template Code
//
// 14-DEC-1998
//


#ifndef __DLGTEMPLATE_H
#define __DLGTEMPLATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define DLGTEMPLATE_BUFFSIZE 32768


///////////////////////////////////////////////////////////////////////////////
//
// Class DlgTemplate
//
class DlgTemplate
{
private:

  U16 buffer[DLGTEMPLATE_BUFFSIZE];
  U16 *ptr;
  U16 *numItems;

  // Align Pointer
  AlignPtr();

public:

  // Constructor & Destructor
  DlgTemplate(const char *title, U16 x, U16 y, U16 width, U16 height, U32 style);
  ~DlgTemplate();

  // AddItem
  void AddItem(
    U16 x, U16 y, 
    U16 width, U16 height, 
    U16 id, U32 style, const char *type, const char *text, U32 exStyle = 0);

  // AddItem
  void AddItem(
    U16 x, U16 y,
    U16 width, U16 height,
    U16 id, U32 style, U16 typeClass, const char *text, U32 exStyle = 0);

  // Get
  void * Get() 
  { 
    return (buffer); 
  }

};

#endif

