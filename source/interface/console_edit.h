/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console Edit Control
//
// 16-MAR-1999
//


#ifndef __CONSOLE_EDIT_H
#define __CONSOLE_EDIT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icedit.h"
#include "console.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ConsoleEdit - Console Edit control
//
class ConsoleEdit : public ICEdit
{
  PROMOTE_LINK(ConsoleEdit, ICEdit, 0x6799DD6A) // "ConsoleEdit"

public:

  // Input modes
  enum ConsoleMode
  {
    EDIT,
    HISTORYSEARCH,
    VARCOMPLETION,
  };

protected:

  // Current input mode
  ConsoleMode mode;

  // Working buffer for recalls and completion
  char workBuf[256];

  // Completion list and iterator
  Console::VCList vcList;
  Console::VCIterator vcItr;

  // History recall list and iterator
  Console::HistList histList;
  Console::HistIterator histItr;

  // Var completion insert point
  S32 insertPos;

  // Associated console viewer
  char *viewerName;
  IControlPtr viewerCtrl;

  // Synchronize work and edit buffers on next dispay
  U32 syncBuffers : 1;

protected:

  // Set input mode
  void SetMode(ConsoleMode m);

  // Begin var completion
  Bool StartCompletion(Bool head = TRUE);

  // Select next or previous element in the var completion list
  void ContinueCompletion(Bool forward = TRUE);

  // Update var completion string
  void UpdateCompletion();

  // End var completion
  void CancelCompletion();

  // Begin history recall
  Bool StartHistRecall(Bool head = TRUE);

  // Select next or previous element in the history recall list
  void ContinueHistRecall(Bool forward = TRUE);

  // Update history recall string
  void UpdateHistRecall();

  // End history recall
  void CancelHistRecall();

  // Synchronize work buffer and input buffer
  void SyncWorkBuf();


  void ResetInputBuf(const char *cmd = NULL);
  void ResetWorkBuf();


public:

  ConsoleEdit(IControl *parent);
  ~ConsoleEdit();

  // Activate the control
  Bool Activate();

  // Deactivate the control
  Bool Deactivate();

  // Draw the control
  void DrawSelf(PaintInfo &pi);

  // Configure the control
  void Setup(FScope *fScope);

  // Event handler
  U32 HandleEvent(Event &e);

};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ConsoleEdit> ConsoleEditPtr;


#endif
