///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Light Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_LIGHTEDITOR_H
#define __STUDIO_LIGHTEDITOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace LightEditorMsg
  //
  namespace LightEditorMsg
  {
    const U32 Copy        = 0x78D47E47; // "Studio::LightEditor::Message::Copy"
    const U32 Add         = 0x2A5934B0; // "Studio::LightEditor::Message::Add"
    const U32 Edit        = 0xCD5A1464; // "Studio::LightEditor::Message::Edit"
    const U32 Grab        = 0xB4C6CA4F; // "Studio::LightEditor::Message::Grab"
    const U32 Delete      = 0x0391643C; // "Studio::LightEditor::Message::Delete"
    const U32 Smooth      = 0xE92F82EE; // "Studio::LightEditor::Message::Smooth"

    const U32 Stop        = 0x11F95C11; // "Studio::LightEditor::Message::Stop"
    const U32 Play        = 0x2F964ECC; // "Studio::LightEditor::Message::Play"
    const U32 FastForward = 0x61C46FEC; // "Studio::LightEditor::Message::FastForward"
    const U32 Reverse     = 0x768564C4; // "Studio::LightEditor::Message::Reverse"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class LightEditor
  //
  class LightEditor : public ICWindow
  {
    PROMOTE_LINK(LightEditor, ICWindow, 0x1EB994CF) // "LightEditor"

  private:

    enum 
    {
      STOPPED,
      PLAYING,
      FASTFORWARDING,
      REVERSING,
    } mode;

    Point<S32> preview;
    S32 previewHeight;

    Point<S32> current;
    S32 currentHeight;

  public:

    // Constructor
    LightEditor(IControl *parent);
    ~LightEditor();
 
    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Event handling
    U32 HandleEvent(Event &e);

    // Control draws itself
    void DrawSelf(PaintInfo &pi);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Poll
    void Poll();

    // Upload
    void Upload();

    // Download
    void Download();
  };

}

#endif
