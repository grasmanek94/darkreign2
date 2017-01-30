/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Static Text Control
//
// 06-JUL-1998
//


#include "icstatic.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "ifvar.h"
#include "input.h"
#include "perfstats.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICStatic
//


//
// ICStatic::ICStatic
//
ICStatic::ICStatic(IControl *parent)
: IControl(parent),
  displayVar(NULL),
  imageVar(NULL),
  staticStyle(0),
  indent(0, 0)
{
  // Default style
  controlStyle |= IControl::STYLE_INERT;
}


//
// ICStatic::~ICStatic
//
ICStatic::~ICStatic()
{
  // Dispose of display var
  if (displayVar)
  {
    delete displayVar;
    displayVar = NULL;
  }

  // Dispose of image var
  if (imageVar)
  {
    delete imageVar;
    imageVar = NULL;
  }

  // Delete images
  images.DisposeAll();

}


//
// ICStatic::Activate
//
// Activate the control
//
Bool ICStatic::Activate()
{
  if (IControl::Activate())
  {
    if (displayVar)
    {
      ActivateVar(displayVar);
    }

    if (imageVar)
    {
      ActivateVar(imageVar, VarSys::VI_STRING);
    }

    if (staticStyle & STYLE_SHEET)
    {
      // Rebuild rectangles
      UpdateSheets();
    }
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICStatic::Deactivate
//
// Deactivate the control
//
Bool ICStatic::Deactivate()
{
  if (IControl::Deactivate())
  {
    if (displayVar)
    {
      displayVar->Deactivate();
    }

    if (imageVar)
    {
      imageVar->Deactivate();
    }

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICStatic::Setup
//
// setup this control using a 'DefineControl' function
//
void ICStatic::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      ConfigureVar(displayVar, fScope);
      break;
    }

    case 0x26186520: // "Images"
    {
      // Build up a tree of images to display keyed to tasks
      FScope *sScope;

      sScope = fScope->GetFunction("UseVar");
      ConfigureVar(imageVar, sScope);

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x76802A4E: // "Image"
          {
            GameIdent name = StdLoad::TypeString(sScope);
            TextureInfo *texture = new TextureInfo;
            IFace::FScopeToTextureInfo(sScope, *texture);
            images.Add(name.crc, texture);
            break;
          }
        }
      }
      break;
    }

    case 0x6979D038: // "Sheet"
    {
      // Specify number images to tile
      IFace::FScopeToSheetInfo(fScope, images, sheets);
      staticStyle |= STYLE_SHEET;
      break;
    }

    default:
    {
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICStatic::SetStyleItem
//
// Change a style setting
//
Bool ICStatic::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x04EDEABF: // "ShowMinSec"
      style = STYLE_SHOWMINSEC;
      break;

    default:
      return IControl::SetStyleItem(s, toggle);
  }

  // Toggle the style
  staticStyle = (toggle) ? (staticStyle | style) : (staticStyle & ~style);

  return TRUE;
}


//
// ICStatic::DrawSelf
//
// Draw the control
//
void ICStatic::DrawSelf(PaintInfo &pi)
{
  TextureInfo *tex = GetTexture();

  if (staticStyle & STYLE_SHEET)
  {
    IFace::RenderSheets(pi.client.p0, images, pi.colors->bg[ColorIndex()], alphaScale);
  }
  else

  if (!(imageVar && !tex))
  {
    DrawCtrlBackground(pi, tex);
  }

  DrawCtrlFrame(pi);

  // If there is a font then draw the control's text
  if (pi.font)
  {
    // Optionally retrieve formatted value from the var
    CH buf[256];
    CH *str = NULL;

    if (displayVar)
    {
      // Does this var require min/sec display
      if ((staticStyle & STYLE_SHOWMINSEC) && (displayVar->Type() == VarSys::VI_FPOINT))
      {
        F32 v = displayVar->GetFloatValue();
        Utils::Sprintf(buf, 128, L"%02u:%02u", (U32)(v / 60.0F), (U32)fmod(v, 60.0F));
      }
      else
      {
        displayVar->GetValue(buf, 256, formatStr);
      }

      str = buf;
    }

    // Draw the text
    DrawCtrlText(pi, str, NULL, indent);
  }
}


//
// ICStatic::Notify
//
// Var changed
//
void ICStatic::Notify(IFaceVar *var)
{
  if (imageVar && var == imageVar)
  {
    SetNamedImage(Crc::CalcStr(imageVar->GetStringValue()));
  }
  else
  {
    IControl::Notify(var);
  }
}


//
// ICStatic::SetNamedImage
//
// Use a named image
//
void ICStatic::SetNamedImage(U32 nameCrc)
{
  SetImage(images.Find(nameCrc));
}


//
// ICStatic::SetDisplayVar
//
// Set the display var
//
void ICStatic::SetDisplayVar(const char *name)
{
  ConfigureVar(displayVar, name);
}


//
// ICStatic::UpdateSheets
//
// Updated sheeted texture coordinates
//
void ICStatic::UpdateSheets()
{
  ASSERT(staticStyle & STYLE_SHEET)
  IFace::UpdateSheets(paintInfo.client, images, sheets);
}
