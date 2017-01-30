/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//

#include "icontrol.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "iface_types.h"
#include "babel.h"
#include "ifvar.h"
#include "input.h"
#include "ptree.h"
#include "console.h"
#include "keybind.h"
#include "vid.h"
#include "main.h"
#include "cursor.h"
#include "fontsys.h"
#include "stdload.h"
#include "operation.h"
#include "comparison.h"
#include "switch.h"


///////////////////////////////////////////////////////////////////////////////
//
// Struct IControlResize - Saved state info for resizing
//
struct IControlResize
{
  IControlPtr control;

  Bool capture;
  Bool focus;
  Bool modal;
};

static IControlResize resizeInfo;


///////////////////////////////////////////////////////////////////////////////
//
// Class IControl - Base class for all interface controls
//


//
// Logging
//
LOGDEF(IControl, "IControl")

// Control names
const char *ToolTipCtrlName = "[ToolTip]";

//
// Static data
//
NList<IControl> IControl::pollList(&IControl::pollNode);
NList<IControl> IControl::deleteList(&IControl::deleteNode);
NList<IControl> IControl::allList(&IControl::allNode);
IControlPtr IControl::previous;


//
// DeleteMarkedControls
//
// Delete all marked items
//
void IControl::DeleteMarkedControls()
{
  for (NList<IControl>::Iterator i(&deleteList); *i;)
  {
    IControl *ctrl = i++;

    // Deactivate the control first
    if (ctrl->IsActive())
    {
      ctrl->Deactivate();
    }
    delete ctrl;
  }
}


//
// Find control by dtrack id
//
IControl * IControl::FindById(U32 id)
{
  for (NList<IControl>::Iterator i(&allList); *i; ++i)
  {
    if ((*i)->dTrack.id == id)
    {
      return (*i);
    }
  }
  return (NULL);
}


//
// Save
//
// Save control info
//
void IControl::Save(FScope *scope)
{
  for (NList<IControl>::Iterator i(&allList); *i; ++i)
  {
    // Get the control
    IControl &c = **i;

    // Do we need to save the activation state
    if (c.controlStyle & STYLE_SAVESTATE)
    {
      FScope *sScope = scope->AddFunction("Activation");
      sScope->AddArgString(c.Name());
      sScope->AddArgInteger(c.IsActive());
    }
  }
}


//
// Load
//
// Save control info
//
void IControl::Load(FScope *scope)
{
  FScope *sScope;

  while ((sScope = scope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xAFBFC0B7: // "Activation"
      {
        if (IControl *c = IFace::FindByName(sScope->NextArgString()))
        {
          c->ChangeActiveState(sScope->NextArgInteger());
        }
        break;
      }
    }
  }
}


//
// IControl::IControl
//
IControl::IControl(IControl *parentCtrl)
: children(&IControl::childNode),
  translations(&EventTranslation::node),
  paintInfo(IFace::data.cgDefault),
  controlStyle(0),
  controlState(0),
  parent(NULL),
  texture(NULL),
  freeText(FALSE),
  freeTipText(FALSE),
  postConfigured(FALSE),
  textJustify(0),
  textStr(NULL),
  formatStr(NULL),
  tipTextStr(NULL),
  inputHook(FALSE),
  forceTextClr(FALSE),
  cursor(0),
  pollInterval(0),
  nextPollTime(0),
  varAlias(NULL),
  region(NULL),
  skin(NULL),
  multiLine(NULL)
{
  // Default size
  SetPos(0, 0);
  SetSize(0, 0);
  memset(&geom, 0, sizeof(geom));

  // Setup parent
  if (parentCtrl)
  {
    SetParent(parentCtrl);
  }

  // Death track registration
  TrackSys::RegisterConstruction(dTrack);

  // Add to the all controls list
  allList.Append(this);
}


//
// IControl::~IControl
//
// Destroys an interface control and all of its children
//
IControl::~IControl()
{
  // Make sure it isnt active
  ASSERT(!IsActive())

  // Delete all children
  DisposeChildren();

  // Detach from parent
  if (parent)
  {
    parent->RemoveChild(this);
    parent = NULL;
  }
  if (region)
  {
    delete region;
  }

  // Destroy lists
  actions.DisposeAll();
  translations.DisposeAll();

  // Delete multiline text manager
  if (multiLine)
  {
    delete multiLine;
    multiLine = NULL;
  }

  // Delete memory
  if (freeText && textStr)
  {
    delete[] textStr;
    textStr = NULL;
  }

  if (formatStr)
  {
    delete[] formatStr;
    formatStr = NULL;
  }

  if (freeTipText && tipTextStr)
  {
    delete[] tipTextStr;
    tipTextStr = NULL;
  }

  // Delete texture info
  if (texture)
  {
    delete texture;
    texture = NULL;
  }

  if (varAlias)
  {
    delete varAlias;
    varAlias = NULL;
  }

  // Register object destruction
  TrackSys::RegisterDestruction(dTrack);

  // Delete local scope
  VarSys::DeleteItem(DynVarName());

  // Remove from the deletion list
  if (deleteNode.InUse())
  {
    deleteList.Unlink(this);
  }

  // Remove from the all controls list
  allList.Unlink(this);
}


//
// IControl::MarkForDeletion
//
// Mark this control for deletion next processing cycle
//
void IControl::MarkForDeletion()
{
  // Allow multiple calls
  if (!deleteNode.InUse())
  {
    deleteList.Append(this);
  }
}


//
// IControl::Draw
//
// Master drawing routine, determines which controls are to be drawn
//
void IControl::Draw(PaintInfo &pi)
{
  if (controlState & STATE_ACTIVE)
  {
    // Apply alpha animation
    if ((controlStyle & STYLE_FADEIN) && (alphaScale < 1.0F))
    {
      // Fade to full alpha over 0.3 seconds
      alphaScale = Min<F32>(alphaScale + F32(IFace::TimeStepMs()) * 0.003F, 1.0F);
      pi.alphaScale *= alphaScale;
    }

    // Draw this control
    DrawSelf(pi);

    // Draw all children
    DrawChildren(pi);
  }
}


//
// IControl::SetParent
//
// Set this control's parent
//
void IControl::SetParent(IControl *ctrl)
{
  ASSERT(ctrl)

  // Is the current parent different to the new one?
  if (parent != ctrl)
  {
    // Yes, is the current parent valid?
    if (parent != NULL)
    {
      // Yes, so remove this control from the parent's list
      parent->RemoveChild(this);
    }

    // Add the
    parent = ctrl;
    parent->AddChild(this);
  }
}


//
// IControl::AddChild
//
// Add a child to this control
//
void IControl::AddChild(IControl *ctrl)
{
  ASSERT(ctrl);

  // Add it to the list
  children.Prepend(ctrl);

  // Propagate the the input hooks flag if TRUE
  if (inputHook)
  {
    ctrl->inputHook = TRUE;
  }
}


//
// IControl::AddChild
//
// Removes specified child from this control
//
void IControl::RemoveChild(IControl *ctrl)
{
  ASSERT(ctrl);

  // Remove it from the list
  children.Unlink(ctrl);
}


//
// IControl::DisposeChildren
//
// Kill all the kiddies
//
void IControl::DisposeChildren()
{
  NList<IControl>::Iterator i(&children);

  while (*i)
  {
    IControl *ctrl = *i;
    i++;

    // Deactivate the control first
    if (ctrl->IsActive())
    {
      ctrl->Deactivate();
    }

    delete ctrl;
  }
}


//
// IControl::EnumChildren
//
// Enumerate list of children
//
void IControl::EnumChildren(Bool (*proc)(IControl *, U32), U32 context)
{
  for (NList<IControl>::Iterator i(&children); *i; i++)
  {
    if (!proc(*i, context))
    {
      return;
    }
  }
}


//
// IControl::ChildCount
//
// Number of children
//
U32 IControl::ChildCount()
{
  return (children.GetCount());
}


//
// IControl::SetTabStop
//
// Finds and sets the tab stop
//
Bool IControl::SetTabStop(IControl *from, Bool forward)
{
  NList<IControl>::Iterator i(&children);

  // If a starting control is specified then find it
  if (from)
  {
    Bool found = FALSE;

    for (; *i; i++)
    {
      if (from == *i)
      {
        if ((*i)->IsTabStop())
        {
          forward ? i-- : i++;
          found = TRUE;
          break;
        }
        else
        {
          LOG_ERR(("'From' [%s] is not a tab stop", from->Name()));
        }
      }
    }

    // Not found, return to tail
    if (!found)
    {
      forward ? i.GoToTail() : i.GoToHead();
    }
  }
  else
  {
    // Otherwise start from the end of the list
    forward ? i.GoToTail() : i.GoToHead();
  }

  // Iterate depending on the direction
  U32 n = children.GetCount();

  while (n--)
  {
    // Move to next element
    if (!(*i))
    {
      forward ? i.GoToTail() : i.GoToHead();
    }

/*
    if (forward)
    {
      if (!(i--))
      {
        i.GoToTail();
      }
    }
    else
    {
      if (!(i++))
      {
        i.GoToHead();
      }
    }
*/

    // Is this a tab stop?
    if ((*i)->IsTabStop())
    {
      (*i)->GetKeyFocus();
      return TRUE;
    }

    forward ? i-- : i++;
  }

  // Couldnt find one
  return (FALSE);
}


//
// IControl::DrawSelf
//
// Draw this control into the specified bitmap
//
void IControl::DrawSelf(PaintInfo &)
{
  // Nothing to do here
}


//
// IControl::DrawChildren
//
// Draw all children of this control into the specified bitmap
//
void IControl::DrawChildren(PaintInfo &pi)
{
  const U32 DISPLAY_MASK = STATE_ACTIVE | STATE_VISIBLE;

  NList<IControl>::Iterator i(&children);

  // Draw children from lowest Z-pos to highest
  for (i.GoToTail(); *i; i--)
  {
    IControl *child = *i;

    if ((child->controlState & DISPLAY_MASK) == DISPLAY_MASK)
    {
      PaintInfo p = child->GetPaintInfo();

      // Convert to screen coordinates before drawing
      Point<S32> origin = child->pos + pi.client.p0;
      p.window += origin;
      p.client += origin;

      // Apply global alpha scale
      p.alphaScale *= IFace::data.alphaScale;

      child->Draw(p);
    }
  }
}


//
// IControl::DrawCtrlText
//
// Draws the text of the object into the previously LOCKED bitmap
//
void IControl::DrawCtrlText(const PaintInfo &pi, const CH *str, Color *colorOverride, const Point<S32> &indent)
{
  // Draw the text
  CH buf[256];

  if (pi.font == NULL)
  {
    ERR_FATAL(("No font specified for [%s]", Name()))
  }

  // Ask the control for a string
  if (multiLine == NULL && str == NULL)
  {
    GetTextString(buf, 256);
    str = buf;
  }

  // Render each line
  U32 lineCount = multiLine ? multiLine->count : 1;
  S32 lineY = 1;

  for (U32 i = 0; i < lineCount; i++)
  {
    // Position the text
    const CH *lineStr;
    S32 lineLen;

    if (multiLine)
    {
      lineStr = multiLine->items[i].text;
      if ((lineLen = multiLine->items[i].length) == 0)
      {
        continue;
      }
    }
    else
    {
      lineStr = str;
      lineLen = Utils::Strlen(lineStr);
    }

    textSize.x = pi.font->Width(lineStr, lineLen);
    textSize.y = pi.font->Height();

    // Align vertically
    if (multiLine)
    {
      textPos.y = lineY;
    }
    else
    {
      if (textJustify & JUSTIFY_TOP)
      {
        textPos.y = 1;
      }
      else

      if (textJustify & JUSTIFY_BOTTOM)
      {
        textPos.y = Max<S32>(pi.client.Height() - S32(textSize.y), 0);
      }
      else
      {
        textPos.y = Max<S32>((pi.client.Height() - textSize.y) / 2, 0);
      }
    }

    // Align horizontally
    if (textJustify & JUSTIFY_LEFT)
    {
      textPos.x = 1;
    }
    else

    if (textJustify & JUSTIFY_RIGHT)
    {
      textPos.x = Max<S32>(pi.client.Width() - S32(textSize.x), 0);
    }
    else
    {
      textPos.x = Max<S32>((pi.client.Width() - S32(textSize.x)) / 2, 0);
    }

    // Add in any indent
    textPos += indent;

    pi.font->Draw
    (
      pi.client.p0.x + textPos.x,
      pi.client.p0.y + textPos.y,
      lineStr,
      lineLen,
      colorOverride ? *colorOverride : (forceTextClr ? forcedTextColor : pi.colors->fg[ColorIndex()]),
      &pi.client,
      pi.alphaScale
    );

    lineY += textSize.y;
  }
}


//
// IControl::DrawCtrlBackground
//
// Redraws the control's background image
//
void IControl::DrawCtrlBackground(const PaintInfo &pi, const TextureInfo *tex)
{
  if (controlStyle & STYLE_TRANSPARENT)
  {
    return;
  }

  // Title bar gradient
  if (controlStyle & STYLE_TITLEGRADIENT)
  {
    if (parent)
    {
      // Draw the Gradient
      IFace::RenderGradient
      (
        pi.client,
        parent->IsModal() ? IFace::GetColor(IFace::CLR_TITLE_BG1) : IFace::GetColor(IFace::CLR_TITLELO_BG1),
        parent->IsModal() ? IFace::GetColor(IFace::CLR_TITLE_BG2) : IFace::GetColor(IFace::CLR_TITLELO_BG2),
        TRUE,
        tex,
        pi.alphaScale
      );
    }
  }
  else

  // Vertical gradient
  if (controlStyle & STYLE_VGRADIENT)
  {
    IFace::RenderGradient
    (
      pi.client, 
      pi.colors->bg[ColorIndex()], 
      IFace::GetMetric(IFace::VGRADIENT), 
      TRUE, 
      tex,
      pi.alphaScale
    );
  }
  else

  // Horizontal gradient
  if (controlStyle & STYLE_HGRADIENT)
  {
    IFace::RenderGradient
    (
      pi.client, 
      pi.colors->bg[ColorIndex()], 
      IFace::GetMetric(IFace::HGRADIENT), 
      FALSE,
      tex,
      pi.alphaScale
    );
  }
  else

  // Plain background
  {
    IFace::RenderRectangle
    (
      pi.client, 
      pi.colors->bg[ColorIndex()], 
      tex,
      pi.alphaScale
    );
  }
}


//
// IControl::DrawCtrlFrame
//
// Draws frame around control
//
void IControl::DrawCtrlFrame(const PaintInfo &pi)
{
  if (skin)
  {
    skin->Render(pi, ColorIndex());
  }
  else
  {
    if (controlStyle & STYLE_THINBORDER)
    {
      IFace::RenderRectangle(pi.window, Color(0L, 0L, 0L, 128L), NULL, pi.alphaScale);
    }
    else

    if (controlStyle & STYLE_THICKBORDER)
    {
      IFace::RenderRectangle(pi.window, Color(0L, 0L, 0L, 128L), NULL, pi.alphaScale);
    }
    else

    if (controlStyle & STYLE_DROPSHADOW)
    {
      IFace::RenderShadow
      (
        pi.client,
        pi.client + (pi.window.p1.y - pi.client.p1.y),
        Color(0L, 0L, 0L, IFace::GetMetric(IFace::SHADOW_ALPHA)),
        IFace::GetMetric(IFace::DROPSHADOW_UP), 
        pi.alphaScale
      );
    }
  }
}


//
// IControl::Setup
//
// Configure this control with one FScope item
//
void IControl::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x70DFC843: // "If"
    {
      if (Comparison::Function(fScope, NULL))
      {
        fScope->InitIterators();
        Configure(fScope);
      }
      else
      {
        if (fScope->ParentFunction())
        {
          FScope *nScope = fScope->ParentFunction()->PeekFunction();
          if (nScope && nScope->NameCrc() == 0x5F3F76C0) // "Else"
          {
            Configure(nScope);
          }
        }
      }
      break;
    }

    case 0x5F3F76C0: // "Else"
      // Ignore elses on their own
      break;

    case 0x4BED1273: // "Switch"
    {
      FSCOPE_DIRTY(fScope)

      // Get the name of the var to test on
      Switch::Value &value = Switch::GetValue(fScope, NULL);

      while (FScope *sScope = fScope->NextFunction())
      {
        if (sScope->NameCrc() == 0x97AF68BF) // "Case"
        {
          if (Switch::CompareValue(value, sScope, NULL))
          {
            Configure(sScope);
            break;
          }
        }
        else if (sScope->NameCrc() == 0x8F651465) // "Default"
        {
          Configure(sScope);
        }
      }

      Switch::DisposeValue(value);
      break;
    }

    case 0xA398E348: // "CreateControl"
    {
      const char *name  = fScope->NextArgString();
      const char *cls   = fScope->NextArgString();

      IControl *ctrl = IFace::CreateControl(name, cls, this);

      if (ctrl == NULL)
      {
        LOG_ERR(("Error creating control [%s]", name));
      }
      else
      {
        ctrl->Configure(fScope);
      }

      // Save this object as the previously configured object
      previous = ctrl;

      break;
    }

    case 0x46E28B78: // "ReadTemplate"
    {
      ReadTemplate(fScope->NextArgString(), TRUE);
      break;
    }

    case 0x63B98E4E: // "ReadRegData"
    {
      FScope *sScope = IFace::FindRegData(fScope->NextArgString(), FALSE);

      if (sScope)
      {
        sScope->InitIterators();
        Configure(sScope);
      }
      break;
    }


    //
    // Position/size/style
    //

    case 0x1FDE7389: // "Style"
    {
      VNode *vNode;

      while ((vNode = fScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
      {
        const char *str = vNode->GetString();
        Bool toggle = TRUE;

        if (*str == '!')
        {
          toggle = FALSE;
          str++;
        }
        SetStyleItem(str, toggle);
      }
      break;
    }

    case 0x09C72CDE: // "Geometry"
    case 0x150D82F6: // "Geom"
    {
      VNode *vNode;

      while ((vNode = fScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
      {
        SetGeometry(vNode->GetString(), NULL);
      }
      break;
    }

    case 0x362FA3AA: // "Size"
    {
      int width  = fScope->NextArgInteger();
      int height = fScope->NextArgInteger();

      SetSize(width, height);
      SetGeomSize(width, height);

      break;
    }

    case 0xCE8A18A6: // "Align"
    {
      // Get the name of the control to align with
      const char *name = fScope->NextArgString();

      if (!SetAlignTo(name))
      {
        LOG_ERR(("Unable to align to control '%s' (not found)", name));
      }
      break;
    }

    case 0x8D878A02: // "Position"
    case 0x29BAA7D3: // "Pos"
    {
      int x = fScope->NextArgInteger();
      int y = fScope->NextArgInteger();

      SetPos(x, y);

      break;
    }

    case 0xB817BF51: // "Region"
    {
      S32 size = StdLoad::TypeU32(fScope);

      // Allocate an array
      region = new Array<Point<S32> >;
      region->Alloc(size);

      FScope *sScope = fScope->NextFunction();;
      for (S32 n = 0; n < size; n++)
      {
        StdLoad::TypePoint<S32>(sScope, (*region)[n]);
        sScope = fScope->NextFunction();
      }
      break;
    }


    //
    // Colors
    //
    case 0x6F3CCD1D: // "ColorGroup"
    {
      const char *name = fScope->NextArgString();
      ColorGroup *c = IFace::FindColorGroup(Crc::CalcStr(name));

      if (c)
      {
        SetColorGroup(c);
      }
      else
      {
        LOG_ERR(("ColorGroup [%s] does not exist [%s]", name, Name()))
      }
      break;
    }

    //
    // Graphics resources
    //

    case 0x7951FC0B: // "Texture"
    case 0x76802A4E: // "Image"
    {
      TextureInfo t;
      IFace::FScopeToTextureInfo(fScope, t);
      SetImage(&t);
      break;
    }

    case 0x7C6FF505: // "Skin"
    {
      SetTextureSkin(IFace::FindTextureSkin(StdLoad::TypeStringCrc(fScope)));
      break;
    }

    case 0xA8C33851: // "Cursor"
    {
      SetCursor(StdLoad::TypeStringCrc(fScope));
      break;
    }

    //
    // Text
    //

    case 0xCB28D32D: // "Text"
    {
      SetTextString(TRANSLATE((StdLoad::TypeString(fScope))), TRUE);
      break;
    }

    case 0x8645C6A0: // "FormatSpec"
    {
      SetFormatSpec(StdLoad::TypeString(fScope));
      break;
    }

    case 0x78D5CFD4: // "Font"
    {
      SetTextFont(StdLoad::TypeString(fScope));
      break;
    }

    case 0x1593179E: // "JustifyText"
    {
      VNode *vNode;

      while ((vNode = fScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
      {
        const char *str = vNode->GetString();

        switch (Crc::CalcStr(str))
        {
          case 0xBA190163: // "Left"
          {
            textJustify &= ~JUSTIFY_HORZ;
            textJustify |= JUSTIFY_LEFT;
            break;
          }

          case 0xE2DDD72B: // "Right"
          {
            textJustify &= ~JUSTIFY_HORZ;
            textJustify |= JUSTIFY_RIGHT;
            break;
          }

          case 0x03633B25: // "Centre"
          case 0x76AB5539: // "Center"
          case 0x97883E31: // "HCentre"
          case 0xE240502D: // "HCenter"      
          {
            textJustify &= ~JUSTIFY_HORZ;
            break;
          }

          case 0x239B3316: // "Top"
          {
            textJustify &= ~JUSTIFY_VERT;
            textJustify |= JUSTIFY_TOP;
            break;
          }

          case 0x5270B6BD: // "Bottom"
          {
            textJustify &= ~JUSTIFY_VERT;
            textJustify |= JUSTIFY_BOTTOM;
            break;
          }

          case 0xA5F1DFCA: // "VCentre"
          case 0xD039B1D6: // "VCenter"
          {
            textJustify &= ~JUSTIFY_VERT;
            break;
          }

          default:
          {
            LOG_ERR(("Unknown Justification [%s] for [%s]", str, Name()));
            break;
          }
        }
      }
      break;
    }

    case 0xF2B876C7: // "TipText"
    {
      SetTipText(TRANSLATE((StdLoad::TypeString(fScope))), TRUE);
      break;
    }

    //
    // Event passing/handling
    //

    case 0x546486D8: // "OnEvent"
    {
      AddEventScope(fScope->NextArgString(), fScope);
      break;
    }

    case 0xFF58B9E9: // "NotifyParent"
    {
      // FIXME: retained for backward compatibility

      // Original event id      
      U32 event  = StdLoad::TypeStringCrc(fScope);

      // Translated event id
      U32 notify = StdLoad::TypeStringCrc(fScope);

      // Optional params
      U32 param1 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);
      U32 param2 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);
      U32 param3 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);

      // Add translation to send events to parent
      AddEventTranslation(event, notify, parent, param1, param2, param3);
     
      break;
    }

    case 0x5E1C2AD5: // "TranslateEvent"
    {
      // Original event id
      U32 event  = StdLoad::TypeStringCrc(fScope);

      // Translated event id
      U32 notify = StdLoad::TypeStringCrc(fScope);

      // Control to send event to
      const char *name = StdLoad::TypeString(fScope);

      // Optional param
      U32 param1 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);
      U32 param2 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);
      U32 param3 = StdLoad::TypeStringCrc(fScope, 0xFFFFFFFF);

      IControl *ctrl = IFace::FindByName(name, this);
      if (ctrl)
      {
        // Add translation to send events to control
        AddEventTranslation(event, notify, ctrl, param1, param2, param3);
      }
      else
      {
        LOG_DIAG(("TranslateEvent: Control [%s] not found", name))
      }
      break;
    }

    case 0x7863C42F: // "PollInterval"
    {
      pollInterval = fScope->NextArgInteger();

      if (pollInterval < 0)
      {
        ERR_FATAL(("Poll interval must not be negative"));
      }
      break;
    }


    //
    // Local variables
    //

    case 0xC2F455BE: // "VarAlias"
    {
      SetVarAlias(StdLoad::TypeString(fScope));
      break;
    }

    case 0x921C808B: // "CreateVarInteger"
    {
      const char *name = StdLoad::TypeString(fScope);
      S32 dVal = S32(StdLoad::TypeU32(fScope, U32(0)));
      S32 minVal = S32(StdLoad::TypeU32(fScope, U32(S32_MIN)));
      S32 maxVal = S32(StdLoad::TypeU32(fScope, U32(S32_MAX)));

      CreateInteger(name, dVal, minVal, maxVal);
      break;
    }

    case 0xAFF1375D: // "CreateVarFloat"
    {
      const char *name = StdLoad::TypeString(fScope);
      F32 dVal = StdLoad::TypeF32(fScope, 0.0F);
      F32 minVal = StdLoad::TypeF32(fScope, F32_MIN);
      F32 maxVal = StdLoad::TypeF32(fScope, F32_MAX);

      CreateFloat(name, dVal, minVal, maxVal);
      break;
    }

    case 0xA2F8DAA2: // "CreateVarString"
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *dVal = StdLoad::TypeStringD(fScope, "");

      CreateString(name, dVal);
      break;
    }

    default:
    {
      // Unknown configuration directive
      LOG_ERR(("Unknown IControl setting [%s] for [%s]", fScope->NameStr(), Name()));
      break;
    }
  }
}


//
// IControl::Configure
//
// Configure this control from an FScope scope
//
void IControl::Configure(FScope *fScope)
{
  FScope *sScope;

  // Pass each function in the scope to the control
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    Setup(sScope);
  }
}


//
// IControl::ReadTemplate
//
// Find and read confuration of a control template
//
Bool IControl::ReadTemplate(const char *name, Bool warn)
{
  ICClass *base = IFace::FindControlClass(Crc::CalcStr(name));

  if (base)
  {
    if (base->scope)
    {
      base->scope->InitIterators();
      Configure(base->scope);
    }
    return (TRUE);
  }

  if (warn)
  {
    LOG_ERR(("ReadTemplate class [%s] not found", name))
  }
  return (FALSE);
}


//
// IControl::PostConfigure
//
// Called after Configure() is completed
//
void IControl::PostConfigure()
{ 
  postConfigured = TRUE;
}


//
// IControl::AutoSize
//
// Automatically resize geometry
//
void IControl::AutoSize()
{
  const U32 TextWidthPad = 3;

  U32 newX = 0, newY = 0;

  if (paintInfo.font)
  {
    ValidateMultiLine();

    if (multiLine)
    {
      if (geom.flags & GEOM_AUTOSIZEX)
      {
        U32 longest = 0;

        // Use longest line
        for (U32 i = 0; i < multiLine->count; i++)
        {
          longest = Max<U32>(paintInfo.font->Width(multiLine->items[i].text, multiLine->items[i].length) + TextWidthPad, longest);
        }
        newX += longest;
      }
      if (geom.flags & GEOM_AUTOSIZEY)
      {
        // Multiply by line height
        newY += (multiLine->count * paintInfo.font->Height());
      }
    }
    else
    {
      CH buf[256];

      // Allow a bit of padding on each side for the DrawText routine
      GetTextString(buf, 256);

      if (geom.flags & GEOM_AUTOSIZEX)
      {
        newX += paintInfo.font->Width(buf, Utils::Strlen(buf)) + TextWidthPad;
      }
      if (geom.flags & GEOM_AUTOSIZEY)
      {
        newY += paintInfo.font->Height();
      }
    }
  }

  // Adjust for border
  ClipRect r = GetAdjustmentRect();
  newX += (r.p0.x - r.p1.x);
  newY += (r.p0.y - r.p1.y);

  size.x = newX;
  size.y = newY;
}


//
// IControl::GetTexture
//
// Return the control's texture info
//
TextureInfo *IControl::GetTexture()
{
  // Is there a specific texture
  if (texture)
  {
    return (texture);
  }

  // Get the color group texture for the current state
  TextureInfo &config = GetPaintInfo().colors->textures[ColorIndex()];

  // Use color group if configured
  return (config.texture ? &config : NULL);
}


//
// IControl::GenerateMultiLine
//
// Generate multiline text
//
void IControl::GenerateMultiLine(const CH *str)
{
  ASSERT(controlStyle & STYLE_MULTILINE)

  const U32 MaxItems = 32;
  MultiLine::Item items[MaxItems];
  U32 count = 0;

  const CH *p = str;
  const CH *startLine = p;
  const CH *endLine = p;
  Bool newLine = FALSE;

  for (;;)
  {
    if (newLine || !*p)
    {
      if (startLine == endLine)
      {
        // Empty line
        items[count].text = startLine;
        items[count].length = 0;
      }
      else
      {
        items[count].text = startLine;
        items[count].length = endLine - startLine + (*p ? 0 : 1);
      }

      count++;

      if (*p)
      {
        startLine = endLine = p;
      }
      else
      {
        break;
      }
      newLine = FALSE;
    }

    endLine = p;

    if ((*p == L'\r') && (*(p + 1) == L'\n'))
    {
      newLine = TRUE;
      p++;
    }
    p++;
  }

  // Build the new structure
  if (multiLine)
  {
    delete multiLine;
  }

  multiLine = new MultiLine(count);

  for (U32 i = 0; i < count; i++)
  {
    multiLine->items[i] = items[i];
  }
}


//
// IControl::ValidateMultiLine
//
// Validate multiline text
//
Bool IControl::ValidateMultiLine()
{
  if (multiLine)
  {
    if (multiLine->items && (multiLine->items[0].text != textStr))
    {
      delete multiLine->items;
      multiLine->items = NULL;
      multiLine->count = 0;

      return (FALSE);
    }
    return (TRUE);
  }
  return (FALSE);
}


//
// SetupAlignment
//
// Setup alignment to another control
//
void IControl::SetupAlignment()
{
  ASSERT(alignTo.Alive());

  // If not active
  if (!alignTo->IsActive())
  {
    // Adjust its geometry
    alignTo->AdjustGeometry();
  }
  /* 
  // Commenting this out will probably break shit
  else
  {
    // Else refresh it
    alignTo->Deactivate();
    alignTo->Activate();
  }
  */

  Point<S32> alignPos(alignTo->pos);
  IControl *ctrl = alignTo->parent ? alignTo->parent : alignTo;
  
  if (!ctrl->IsChild(this))
  {
    alignPos = alignTo->ClientToScreen(Point<S32>(0, 0));
  }

  if (geom.flags & GEOM_ALIGNTOWIDTH)
  {
    size.x = alignTo->size.x;
  }

  if (geom.flags & GEOM_ALIGNTOHEIGHT)
  {
    size.y = alignTo->size.y;
  }

  if (geom.flags & GEOM_RIGHT)
  {
    pos.x = alignPos.x + alignTo->size.x + geom.pos.x;

    if (geom.flags & GEOM_HINTERNAL)
    {
      pos.x -= size.x;
    }
  }
  else
  {
    pos.x = alignPos.x + geom.pos.x - ((geom.flags & GEOM_HINTERNAL) ? 0 : size.x);
  }

  if (geom.flags & GEOM_BOTTOM)
  {
    pos.y = alignPos.y + alignTo->size.y + geom.pos.y;

    if (geom.flags & GEOM_VINTERNAL)
    {
      pos.y -= size.y;
    }
  }
  else
  {
    pos.y = alignPos.y + geom.pos.y - ((geom.flags & GEOM_VINTERNAL) ? 0 : size.y);
  }
}


//
// IControl::AdjustGeometry
//
// Adjust geometry of control
//
void IControl::AdjustGeometry()
{
  Point<S32> oldSize = size;

  if (parent == NULL)
  {
    return;
  }

  // Should we autosize this control
  if (geom.flags & GEOM_AUTOSIZE)
  {
    AutoSize();

    // Add on user values
    size.x += geom.size.x;
    size.y += geom.size.y;
  }

  // Parent width
  if (geom.flags & GEOM_PARENTWIDTH)
  {
    size.x = parent->GetPaintInfo().client.Width() + geom.size.x;
  }

  if (geom.flags & GEOM_WINPARENTWIDTH)
  {
    size.x = parent->GetPaintInfo().window.Width() + geom.size.x;
  }

  // Parent height
  if (geom.flags & GEOM_PARENTHEIGHT)
  {
    size.y = parent->GetPaintInfo().client.Height() + geom.size.y;
  }

  if (geom.flags & GEOM_WINPARENTHEIGHT)
  {
    size.y = parent->GetPaintInfo().window.Height() + geom.size.y;
  }

  if (geom.flags & GEOM_SQUARE)
  {
    size.x = Min(size.x, size.y);
    size.y = size.x;
  }

  // Are we aligning to another control
  if (alignTo.Alive())
  {
    SetupAlignment();
  }
  else
  {
    // Horizontal position
    if (geom.flags & GEOM_RIGHT)
    {
      pos.x = parent->GetPaintInfo().client.Width() - size.x + geom.pos.x;
    }

    if (geom.flags & GEOM_WINLEFT)
    {
      pos.x = parent->GetPaintInfo().window.p0.x - parent->GetPaintInfo().client.p0.x;
    }

    if (geom.flags & GEOM_WINRIGHT)
    {
      pos.x = parent->GetPaintInfo().window.p1.x - parent->GetPaintInfo().client.p0.x - size.x + geom.pos.x;
    }

    // Vertical position
    if (geom.flags & GEOM_BOTTOM)
    {
      pos.y = parent->GetPaintInfo().client.Height() - size.y + geom.pos.y;
    }

    if (geom.flags & GEOM_WINTOP)
    {
      pos.y = parent->GetPaintInfo().window.p0.y - parent->GetPaintInfo().client.p0.y;
    }

    if (geom.flags & GEOM_WINBOTTOM)
    {
      pos.y = parent->GetPaintInfo().window.p1.y - parent->GetPaintInfo().client.p0.y - size.y + geom.pos.y;
    }
  }

  if (geom.flags & GEOM_VCENTRE)
  {
    pos.y = (parent->GetPaintInfo().client.Height() - size.y) / 2 + geom.pos.y;
  }

  if (geom.flags & GEOM_WINVCENTRE)
  {
    pos.y = ((parent->GetPaintInfo().window.Height() - size.y) / 2) - parent->GetPaintInfo().client.p0.y + geom.pos.y;
  }

  if (geom.flags & GEOM_HCENTRE)
  {
    pos.x = (parent->GetPaintInfo().client.Width() - size.x) / 2 + geom.pos.x;
  }

  if (geom.flags & GEOM_WINHCENTRE)
  {
    pos.x = ((parent->GetPaintInfo().window.Width() - size.x) / 2) - parent->GetPaintInfo().client.p0.x + geom.pos.x;
  }

  // If required, ensure control is entirely visible
  if (geom.flags & GEOM_KEEPVISIBLE)
  {
    pos.x = Clamp<S32>(0, pos.x, parent->size.x - size.x);
    pos.y = Clamp<S32>(0, pos.y, parent->size.y - size.y);
  }

  // Adjust PaintInfo parameters
  paintInfo.window.Set(0, 0, size.x, size.y);

  // Adjust for border
  paintInfo.client = paintInfo.window + GetAdjustmentRect();
}


//
// Calculate an adjustment rect based on the style
//
ClipRect IControl::GetAdjustmentRect()
{
  ClipRect r;

  if (skin)
  {
    r = skin->border;
  }
  else
  {
    // Adjust for border
    if (controlStyle & STYLE_THINBORDER)
    {
      S32 border = IFace::GetMetric(IFace::BORDER_THIN);
      r.Set(border, border, -border, -border);
    }
    else

    if (controlStyle & STYLE_THICKBORDER)
    {
      S32 border = IFace::GetMetric(IFace::BORDER_THICK);
      r.Set(border, border, -border, -border);
    }
    else

    if (controlStyle & STYLE_DROPSHADOW)
    {
      S32 shadow = IFace::GetMetric(IFace::DROPSHADOW_UP);
      r.Set(0, 0, -shadow, -shadow);
    }

    else
    {
      r.Set(0, 0, 0, 0);
    }
  }

  return (ClipRect(r));
}


//
// IControl::CanActivate
//
// Ask a control if it can activate
//
Bool IControl::CanActivate()
{
  // Is it already active?
  if (controlState & STATE_ACTIVE)
  {
    return (FALSE);
  }

  // Is it marked for deletion?
  if (deleteNode.InUse())
  {
    return (FALSE);
  }

  // It can activate
  return (TRUE);
}


//
// IControl::Activate
//
// Create the window surface and load its images
//
Bool IControl::Activate()
{
  if (!postConfigured)
  {
    // Notify that configuration is complete, only once per lifetime, 
    // and before first activation
    PostConfigure();
    ASSERT(postConfigured)
  }

  if (!CanActivate())
  {
    return (FALSE);
  }

  // Notify control that it is activating
  SendNotify(this, IControlNotify::Activating);

  // Adjust geometry
  AdjustGeometry();

  // Add to poll list if necessary
  if (pollInterval)
  {
    AddToPollList();
  }

  // Activate all children
  NList<IControl>::Iterator i(&children);

  for (i.GoToTail(); *i; i--)
  {
    IControl *p = *i;

    if (!(p->GetControlStyle() & STYLE_NOAUTOACTIVATE))
    {
      p->Activate();
    }
  }

  // Activate and make it visible
  controlState |= STATE_ACTIVE | STATE_VISIBLE;

  // Update texture coordinates for tiled image
  if (texture)
  {
    texture->UpdateUV(paintInfo.client);
  }

  // If modal flag is set, bring window to the top
  if (controlStyle & STYLE_MODAL)
  {
    // Set the modal flag
    SetModal();

    // Activate the first item on the tab list
    SetTabStop(NULL, TRUE);
  }
  else

  if (controlStyle & STYLE_SETTABSTOP)
  {
    // Activate the first item on the tab list
    SetTabStop(NULL, TRUE);
  }

  // Alpha animation
  if (controlStyle & STYLE_FADEIN)
  {
    alphaScale = 0.0F;
  }
  else
  {
    alphaScale = 1.0F;
  }

  // Notify control that is was activated
  SendNotify(this, IControlNotify::Activated);

  return (TRUE);
}


//
// IControl::Deactivate
//
Bool IControl::Deactivate()
{
  if (!(controlState & STATE_ACTIVE))
  {
    return FALSE;
  }

  // Notify control that it is deactivating
  SendNotify(this, IControlNotify::Deactivating);

  // Remove from poll list if necessary
  if (pollNode.InUse())
  {
    RemoveFromPollList();
  }

  // Release focus/capture etc
  if (HasKeyFocus())
  {
    ReleaseKeyFocus();
  }
  if (HasMouseCapture())
  {
    ReleaseMouseCapture();
  }
  if (IsModal())
  {
    UnsetModal();
  }

  // Deactivate all children
  for (NList<IControl>::Iterator i(&children); *i; i++)
  {
    (*i)->Deactivate();
  }

  // Update state
  controlState &= ~STATE_ACTIVE;

  if (texture && texture->texture && texture->texture->GetBink() && texture->texture->GetStatus().binkActive)
  {
    // stop movie playback
    //
    texture->texture->BinkSetActive( FALSE);
  }

  // Notify control that is was deactivated
  SendNotify(this, IControlNotify::Deactivated);

  return (TRUE);
}


//
// IControl::ToggleActive
//
// Toggle the active/inactive state of this control
//
Bool IControl::ToggleActive()
{
  if (controlState & STATE_ACTIVE)
  {
    return Deactivate();
  }
  else
  {
    return Activate();
  }
}



//
// IControl::ChangeActiveState
//
// If flag is true, call Activate, otherwise call Deactivate
//
Bool IControl::ChangeActiveState(Bool flag)
{
  return (flag ? Activate() : Deactivate());
}


//
// IControl::MoveTo
//
// Move control to position
//
void IControl::MoveTo(const Point<S32> &p)
{
  // If we have the geometry style keep visible, clamp to on screen
  if (geom.flags & GEOM_KEEPVISIBLE)
  {
    if (p.x + size.x > IFace::ScreenWidth())
    {
      pos.x = IFace::ScreenWidth() - size.x;
    }
    else if (p.x < 0)
    {
      pos.x = 0;
    }
    else
    {
      pos.x = p.x;
    }

    if (p.y + size.y > IFace::ScreenHeight())
    {
      pos.x = IFace::ScreenHeight() - size.y;
    }
    else if (p.y < 0)
    {
      pos.y = 0;
    }
    else
    {
      pos.y = p.y;
    }
  }
  else
  {
    pos = p;
  }
}


//
// IControl::Resize
//
// Resize control
//
void IControl::Resize(const Point<S32> &p)
{
  Bool active = IsActive();

  // Deactivate first
  if (active)
  {
    SendNotify(this, IControlNotify::PreResize, FALSE);
    Deactivate();
  }

  // Resize
  size = p;

  // Reactivate if necessary
  if (active)
  {
    Activate();
    SendNotify(this, IControlNotify::PostResize, FALSE);
  }
}


//
// IControl::SetZPos
//
// Set Z position of this control
//
void IControl::SetZPos(U32 flags)
{
  /*if (flags & topmost)*/
  {
    if (parent)
    {
      NList<IControl> &list = parent->children;

      if (childNode.InUse())
      {
        list.Unlink(this);

        if (flags == 0xFFFFFFFF)
        {
          list.Append(this);
        }
        else
        {
          IControl *modalCtrl = IFace::GetModal();

          // Do we need to move the modal
          Bool moveModal = !(controlStyle & STYLE_MODAL) && (modalCtrl && (modalCtrl != this) && (modalCtrl->parent == parent));

          if (moveModal)        
          {
            list.Unlink(modalCtrl);
          }

          list.Prepend(this);

          if (moveModal)
          {
            list.Prepend(modalCtrl);
          }
        }
      }
    }
  }
}


//
// Activate Tip
//
void IControl::ActivateTip(const CH *text)
{
  if (text)
  {
    IControl *tipParent = IFace::OverlaysWindow();
    IControl *c = IFace::CreateControl(ToolTipCtrlName, 0x7F35ADA0, tipParent); // "Sys::ToolTip"

    if (c)
    {
      // Setup iface with the control
      IFace::SetTipControl(c);

      // Setup the tip window
      c->SetTextString(text, FALSE);

      // Move it to the mouse cursor position
      c->pos += Input::MousePos();

      // Let geometry take care of the rest
      c->Activate();
    }
  }
}


//
// IControl::HandleEvent
//
U32 IControl::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        switch (e.input.code)
        {
          case DIK_TAB:
          {
            // Tab and Shift-Tab move through the TabStop list
            if (e.input.state & Input::SHIFTDOWN)
            {
              // Generate a prevtab message
              SendNotify(parent, IControlMsg::PrevTabStop, FALSE);
              return TRUE;
            }
            else
            {
              // Generate a nexttab message
              SendNotify(parent, IControlMsg::NextTabStop, FALSE);
              return TRUE;
            }
            break;
          }
        }
        break;
      }
    }
  }
  else

  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // A child control is notifying us of an event
      case IFace::NOTIFY:
      {
        U32 id = e.iface.p1;

        // Handle the event
        switch (id)
        {
          case IControlMsg::PrevTabStop:
          {
            // Move to next tab
            SetTabStop(IFace::GetFocus(), FALSE);

            // Handled
            return (TRUE);
          }

          case IControlMsg::NextTabStop:
          {
            // Move to previous tab
            SetTabStop(IFace::GetFocus(), TRUE);

            // Handled
            return (TRUE);
          }

          case IControlNotify::PreResize:
          {
            ASSERT(IsActive())

            resizeInfo.control = this;
            resizeInfo.capture = HasMouseCapture();
            resizeInfo.focus   = HasKeyFocus();
            resizeInfo.modal   = IsModal();

            // Handled
            return (TRUE);
          }

          case IControlNotify::PostResize:
          {
            if (resizeInfo.control.Alive() && (resizeInfo.control.GetData() == this))
            {
              // Restore state
              if (resizeInfo.capture)
              {
                GetMouseCapture();
              }
              if (resizeInfo.focus)
              {
                GetKeyFocus();
              }
              if (resizeInfo.modal)
              {
                SetModal();
              }
            }

            // Clear reaper
            resizeInfo.control = NULL;

            // Handled
            return (TRUE);
          }

          default:
          {
            // Execute a custom action scope, see AddEventScope()
            FScope *actionScope;

            if ((actionScope = actions.Find(id)) != NULL)
            {
              ExecScope(actionScope);
            }

            // Handled
            return (TRUE);
          }
        }

        // Not handled
        break;
      }

      // The display mode has changed
      case IFace::DISPLAYMODECHANGED:
      {
        if (controlState & STATE_ACTIVE)
        {
          // Notify all children
          for (NList<IControl>::Iterator i(&children); *i; i++)
          {
            IFace::SendEvent(*i, NULL, IFace::DISPLAYMODECHANGED, e.iface.p1, e.iface.p2);
          }
        }

        // Handled
        return (TRUE);
      }

      // Mouse moved in or out of control
      case IFace::MOUSEIN:
      {
        controlState |= STATE_HILITE;
        return (TRUE);
      }

      case IFace::MOUSEOUT:
      {
        controlState &= ~STATE_HILITE;
        return (TRUE);
      }

      case IFace::DISPLAYTIP:
      {
        ActivateTip(GetTipTextString());
        return (TRUE);
      }

      case IFace::TIPDELAY:
      {
        return (U32(IFace::GetMetric(IFace::TIP_DELAY)));
      }
    }
  }

  // Not handled
  return FALSE;
}


//
// IControl::Find
//
// Find a child control at screen position x,y
//
IControl *IControl::Find(S32 x, S32 y, Bool all)
{
  if (controlState & STATE_ACTIVE)
  {
    Bool in;

    if (region)
    {
      in = Point<S32>(x - pos.x, y - pos.y).IsInPolyConvex(region->data, region->count);
    }
    else
    {
      in = ((x >= pos.x) && (y >= pos.y) && (x < pos.x + size.x) && (y < pos.y + size.y));
    }

    if (in)
    {
      // x,y is inside this control, is it inside any children?
      for (NList<IControl>::Iterator i(&children); *i; i++)
      {
        IControl *found = NULL;

        if ((found = (*i)->Find(x - pos.x - paintInfo.client.p0.x, y - pos.y - paintInfo.client.p0.y, all)) != NULL)
        {
          return (found);
        }
      }

      // No, return this
      if (all)
      {
        return (this);
      }
      else
      {
        return (controlStyle & STYLE_INERT ? NULL : this);
      }
    }
  }
  return (NULL);
}


//
// IControl::Find
//
// Find a child control by CRC of name
//
IControl *IControl::Find(U32 id, Bool descend)
{
  if (children.GetCount() > 0)
  {
    NList<IControl>::Iterator i(&children);

    for (!i; *i; i++)
    {
      if ((*i)->ID() == id)
      {
        return (*i);
      }
    }

    if (descend)
    {
      for (!i; *i; i++)
      {
        IControl *found;

        if ((found = (*i)->Find(id, descend)) != NULL)
        {
          return found;
        }
      }
    }
  }
  return NULL;
}


//
// IControl::IsChild
//
// Test if a control is a child of this control
//
Bool IControl::IsChild(IControl *ctrl)
{
  if (ctrl == this)
  {
    return TRUE;
  }
  else
  {
    for (NList<IControl>::Iterator i(&children); *i; i++)
    {
      if ((*i)->IsChild(ctrl))
      {
        return TRUE;
      }
    }
    return FALSE;
  }
}


//
// IControl::Poll
//
// Callback function for controls that are on the poll list
//
void IControl::Poll()
{
}


//
// IControl::Notify
//
// Function called when a var being watched changes value
//
void IControl::Notify(IFaceVar *)
{
}


//
// IControl::ConfigureVar
//
// Configure an iface var
//
void IControl::ConfigureVar(IFaceVar * &var, FScope *fScope)
{
  ASSERT(fScope)

  // Ensure var is not already setup
  if (var)
  {
    fScope->ScopeError("Var already setup for [%s]", Name());
  }

  // Allocate the var
  var = new IFaceVar(this, FindVarName(fScope->NextArgString()));
}


//
// IControl::ConfigureVar
//
// Configure an iface var
//
void IControl::ConfigureVar(IFaceVar * &var, const char *name)
{
  // Ensure var is not already setup
  if (var)
  {
    ERR_FATAL(("Var already setup for [%s]", Name()));
  }

  // Allocate the var
  var = new IFaceVar(this, FindVarName(name));
}


//
// IControl::ActivateVar
//
// Activate and optionally check the type of the var
//
void IControl::ActivateVar(IFaceVar *var, VarSys::VarItemType type)
{
  // Ensure var exists
  if (var == NULL)
  {
    ERR_FATAL(("Var not specified for [%s]", Name()));
  }

  // Set it up
  var->Activate();

  // ..and has been setup
  if (!var->IsValid() || (var->Type() == VarSys::VI_NONE))
  {
    ERR_FATAL(("Var not setup for [%s]", Name()));
  }

  // ..and that its type matches
  if ((type != VarSys::VI_NONE) && (var->Type() != type))
  {
    ERR_FATAL(("%s var expected for [%s]", VarSys::GetTypeString(type), Name()));
  }
}


//
// IControl::SetEnabled
//
// Set the enabled state
//
void IControl::SetEnabled(Bool flag, Bool recurse)
{
  if (flag)
  {
    controlState &= ~STATE_DISABLED;
  }
  else
  {
    controlState |= STATE_DISABLED;
  }

  if (recurse)
  {
    for (NList<IControl>::Iterator i(&children); *i; i++)
    {
      (*i)->SetEnabled(flag, recurse);
    }
  }
}


//
// IControl::SetVarAlias
//
void IControl::SetVarAlias(const char *alias)
{
  if (varAlias)
  {
    delete varAlias;
  }
  varAlias = Utils::Strdup(alias);
}


//
// IControl::DynVarName
//
// Construct a name of a dynamic variable
//
const char *IControl::DynVarName(const char *var)
{
  static char buf[VARSYS_MAXVARPATH];
  const char *p;
  char *s = buf;

  if (varAlias)
  {
    // Copy var alias
    p = varAlias;
    while (*p)
    {
      *s++ = *p++;
    }
  }
  else
  {
    // Copy scope name
    p = IFace::DYNAMICDATA_SCOPE;
    while (*p)
    {
      *s++ = *p++;
    }
    *s++ = VARSYS_SCOPEDELIM;

    // Copy dtrack id as hex value
    Utils::StrFmtHex(s, 8, dTrack.id);
    s += 8;
  }

  // Copy optional var name
  if (var)
  {
    *s++ = VARSYS_SCOPEDELIM;
    p = var;

    while (*p)
    {
      *s++ = *p++;
    }
  }

  // Null terminate
  *s = '\0';

  return buf;
}


//
// IControl::FindVarName
//
// Find a var name.
//
// This uses the syntax of ICRoot::FindByName
// If the var is prepended by '$' then
//
// example:
//  $var          - replaced with var in current control var's scope
//  var           - returns var
//  $|var         - replaced with var in root control var scope
//  $<sibling.var - replaced with var in sibling
//  $<.var        - replaced with var in parent
//
const char *IControl::FindVarName(const char *var)
{
  static char buf[VARSYS_MAXVARPATH];

  // Perform the substiution
  if (*var == '$')
  {
    IControl *ctrl = this;
    const char *stripped = var + 1;

    char *dot = Utils::Strrchr(stripped, '.');

    if (dot)
    {
      // Copy the control name portion
      Utils::Strmcpy(buf, stripped, dot - stripped + 1);

      // Find the control
      ctrl = IFace::FindByName(buf, this);

      if (ctrl == NULL)
      {
        LOG_DIAG(("FindVarName: could not find control [%s]", buf));
        return (var);
      }

      // Construct the var name
      return (ctrl->DynVarName(dot + 1));
    }
    else
    {
      return (ctrl->DynVarName(stripped));
    }
  }
  else
  {
    // Else return the unmodified var name
    return (var);
  }
}


//
// CreateInteger
//
// Create an integer local var
//
VarSys::VarItem * IControl::CreateInteger(const char *name, S32 dVal, S32 low, S32 high)
{
  VarSys::VarItem *item = VarSys::CreateInteger(DynVarName(name), dVal);
  item->SetIntegerRange(low, high);
  return (item);
}


//
// CreateFloat
//
// Create a float local var
//
VarSys::VarItem * IControl::CreateFloat(const char *name, F32 dVal, F32 low, F32 high)
{
  VarSys::VarItem *item = VarSys::CreateFloat(DynVarName(name), dVal);
  item->SetFloatRange(low, high);
  return (item);
}



//
// CreateString
//
// Create a string local var
//
VarSys::VarItem * IControl::CreateString(const char *name, const char *dVal)
{
  return (VarSys::CreateString(DynVarName(name), dVal));
}


//
// IControl::InWindow
//
// Returns TRUE if the point p (in screen co-ordinates) is inside this window
//
Bool IControl::InWindow(const Point<S32> &p) const
{
  Point<S32> wnd = ScreenToWindow(p);

  if (region)
  {
    return (wnd.IsInPolyConvex(region->data, region->count));
  }
  else
  {
    return ((wnd.x >= 0) && (wnd.y >= 0) && (wnd.x <= size.x) && (wnd.y <= size.y));
  }
}


//
// IControl::InClient
//
Bool IControl::InClient(const Point<S32> &p) const
{
  return InClientFromWindow(ScreenToClient(p));
}


//
// IControl::InClientFromWindow
//
Bool IControl::InClientFromWindow(const Point<S32> &p) const
{
  return (paintInfo.client.In(p));
}


//
// Convert the screen co-ordinates 'p' to a position relative to the window
//
Point<S32> IControl::ScreenToWindow(const Point<S32> &p) const
{
  const IControl *ctrl = this;
  Point<S32> wnd = p - ctrl->pos;

  // NOTE: paintInfo.window should always be (0,0)
  while ((ctrl = ctrl->parent) != NULL)
  {
    wnd -= (ctrl->pos + ctrl->GetPaintInfo().client.p0);
  }
  return (wnd);
}


//
// Convert the screen co-ordinates 'p' to a position relative to the client
//
Point<S32> IControl::ScreenToClient(const Point<S32> &p) const
{
  // NOTE: paintInfo.window should always be (0,0)
  return (ScreenToWindow(p) - paintInfo.client.p0);
}


//
// Convert client coordinates to screen coordinates
//
Point<S32> IControl::ClientToScreen(const Point<S32> &p) const
{
  const IControl *ctrl = this;
  Point<S32> cli = p;

  while (ctrl)
  {
    cli += ctrl->pos + ctrl->GetPaintInfo().client.p0;
    ctrl = ctrl->parent;
  }

  return cli;
}


//
// IControl::GetMouseCapture
//
// Capture the mouse
//
void IControl::GetMouseCapture()
{
  IFace::SetMouseCapture(this); 
}


//
// IControl::ReleaseMouseCapture
//
// Release mouse
//
void IControl::ReleaseMouseCapture()
{
  IFace::ReleaseMouseCapture(this); 
}


//
// IControl::HasMouseCapture
//
// Test if this control has capture
//
Bool IControl::HasMouseCapture()
{
  return (IFace::GetCapture() == this ? TRUE : FALSE);
}


//
// IControl::GetKeyFocus
//
// Get keyboard focus
//
void IControl::GetKeyFocus()
{
  IFace::SetFocus(this);
}


//
// IControl::ReleaseKeyFocus
//
// Release keyboard focus
//
void IControl::ReleaseKeyFocus()
{
  IFace::ReleaseFocus(this);
}


//
// IControl::HasKeyFocus
//
// Test if this control has focus
//
Bool IControl::HasKeyFocus()
{
  return (IFace::GetFocus() == this ? TRUE : FALSE);
}


//
// IControl::SetModal
//
// Set modal state
//
void IControl::SetModal()
{
  IFace::SetModal(this);
}


//
// IControl::UnsetModal
//
// Unset modal state
//
void IControl::UnsetModal()
{
  IFace::UnsetModal(this);
}


//
// IControl::IsModal
//
// Test if this control is modal
//
Bool IControl::IsModal()
{
  return (IFace::GetModal() == this ? TRUE : FALSE);
}


//
// IControl::IsMouseOver
//
// Is the cursor over this control
//
Bool IControl::IsMouseOver()
{
  return ((IFace::GetMouseOver() == this) ? TRUE : FALSE);
}


///////////////////////////////////////////////////////////////////////////////
//
// Control configuration directives
//


//
// IControl::SetName
//
void IControl::SetName(const char *name) 
{ 
  ASSERT(name);

// FIXME : This is valid, but we needed to take it out 
// to get the terrain listbox hack working...DOH!
/*
  if (Utils::Strchr(name, VARSYS_SCOPEDELIM))
  {
    ERR_FATAL(("Control name cannot contain '%c'", VARSYS_SCOPEDELIM));
  }
*/

  ident = name; 
}


//
// IControl::SetStyle
//
// Set the style for this control, arg list must be NULL terminated
//
void CDECL IControl::SetStyle(const char *s, ...)
{
  va_list args;
  const char *str = s;
  U32 count = 0;

  va_start(args, s);

  while (str)
  {
    Bool toggle = TRUE;

    if (*str == '!')
    {
      toggle = FALSE;
      str++;
    }
    SetStyleItem(str, toggle);

    // Next string
    str = va_arg(args, const char *);
    count++;

#ifdef DEVELOPMENT
    if (count == 100)
    {
      ERR_FATAL(("SetStyle: args not null terminated?"));
    }
#endif
  }

  va_end(args);
}


//
// IControl::SetStyleItem
//
// Change a style setting
//
Bool IControl::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0xD07BB6DB: // "Transparent"
      style = STYLE_TRANSPARENT;
      break;

    case 0xD1F1A181: // "Border"
    case 0x85682BDC: // "ThinBorder"
      style = STYLE_THINBORDER;
      break;

    case 0x8E2D3B74: // "SunkenBorder"
      style = STYLE_SUNKENBORDER;
      break;

    case 0x8391F316: // "ThickBorder"
      style = STYLE_THICKBORDER;
      break;

    case 0xCF4762D7: // "DropShadow"
      style = STYLE_DROPSHADOW;
      break;

    case 0x2BCBD026: // "Modal"
      style = STYLE_MODAL;
      break;

    case 0x130C47D7: // "ModalClose"
      SetStyleItem("Modal", TRUE);
      style = STYLE_MODALCLOSE;
      break;

    case 0xA2139C2B: // "TitleGradient"
      style = STYLE_TITLEGRADIENT;
      break;

    case 0xFB39354A: // "Gradient"   
    case 0x74B064A2: // "VGradient"
      style = STYLE_VGRADIENT;
      break;

    case 0xC736BA70: // "HGradient"
      style = STYLE_HGRADIENT;
      break;

    case 0x0E7AD538: // "TabStop"
      style = STYLE_TABSTOP;
      break;

    case 0x75B5764D: // "NoAutoActivate"
      style = STYLE_NOAUTOACTIVATE;
      break;

    case 0xCE0418E9: // "FadeIn"
      style = STYLE_FADEIN;
      break;

    case 0x163A9EFE: // "SetTabStop"
      style = STYLE_SETTABSTOP;
      break;

    case 0xD0A3B705: // "MultiLine"
      style = STYLE_MULTILINE;
      break;

    case 0x68C66A0B: // "SaveState"
      style = STYLE_SAVESTATE;
      break;

    case 0xDF054928: // "SystemWide"
      style = STYLE_SYSTEMWIDE;
      break;

    default:
      LOG_ERR(("Unknown Control Style [%s] for [%s]", s, Name()));
      return FALSE;
  }

  // Toggle the style
  controlStyle = (toggle) ? (controlStyle | style) : (controlStyle & ~style);

  return TRUE;
}


//
// IControl::SetStateItem
//
// Change a style setting
//
Bool IControl::SetStateItem(const char *s, Bool toggle)
{
  U32 state;

  switch (Crc::CalcStr(s))
  {
    case 0x38652912: // "Selected"
      state = STATE_SELECTED;
      break;

    case 0xDA7FE644: // "Hilite"
      state = STATE_HILITE;
      break;

    case 0x98B375EE: // "Disabled"
      state = STATE_DISABLED;
      break;

    case 0x71BB5A64: // "MaskColor"
      state = STATE_MASK_COLOR;
      break;

    // Active
    case 0x65E86346: // "Active"
      state = STATE_ACTIVE;
      break;

    // Visible
    case 0x01C95681: // "Visible"
      state = STATE_VISIBLE;
      break;

    default:
      LOG_ERR(("Unknown Control State [%s] for [%s]", s, Name()));
      return FALSE;
  }

  // Toggle the style
  controlState = (toggle) ? (controlState | state) : (controlState & ~state);

  return TRUE;
}


//
// IControl::SetGeometry
//
// Set the geometry properties of this control
//
void CDECL IControl::SetGeometry(const char *s, ...)
{
  const char *str = s;
  va_list args;

  va_start(args, s);

  while (str)
  {
    SetGeometryItem(str, TRUE);
    str = va_arg(args, const char *);
  }

  va_end(args);
}



//
// IControl::SetGeometryItem
//
// Change a geometry setting
//
Bool IControl::SetGeometryItem(const char *s, Bool toggle)
{
  U32 geometry;

  switch (Crc::CalcStr(s))
  {
    case 0x239B3316: // "Top"
      geometry = GEOM_BOTTOM;
      toggle = !toggle;
      break;

    case 0xBA190163: // "Left"
      geometry = GEOM_RIGHT;
      toggle = !toggle;
      break;

    case 0xE2DDD72B: // "Right"
      geometry = GEOM_RIGHT;
      break;

    case 0x5270B6BD: // "Bottom"
      geometry = GEOM_BOTTOM;
      break;

    case 0x97883E31: // "HCentre"
      geometry = GEOM_HCENTRE;
      break;

    case 0xA5F1DFCA: // "VCentre"
      geometry = GEOM_VCENTRE;
      break;

    case 0x31A85A6B: // "ParentWidth"
      geometry = GEOM_PARENTWIDTH;
      break;

    case 0x33BB440C: // "ParentHeight"
      geometry = GEOM_PARENTHEIGHT;
      break;

    case 0x22A55929: // "WinLeft"
      geometry = GEOM_WINLEFT;
      break;

    case 0x5D904B0D: // "WinRight"
      geometry = GEOM_WINRIGHT;
      break;

    case 0xEE6FC858: // "WinTop"
      geometry = GEOM_WINTOP;
      break;

    case 0x9A9DA07E: // "WinBottom"
      geometry = GEOM_WINBOTTOM;
      break;

    case 0x011C8010: // "WinHCentre"
    case 0x74D4EE0C: // "WinHCenter"
      geometry = GEOM_WINHCENTRE;
      break;

    case 0x336561EB: // "WinVCentre"
    case 0x46AD0FF7: // "WinVCenter"
      geometry = GEOM_WINVCENTRE;
      break;

    case 0x170935EB: // "WinParentWidth"
      geometry = GEOM_WINPARENTWIDTH;
      break;

    case 0x10713F5E: // "WinParentHeight"
      geometry = GEOM_WINPARENTHEIGHT;
      break;

    case 0x63BDADB1: // "Square"
      geometry = GEOM_SQUARE;
      break;

    case 0x33888D96: // "AutoSize"
      geometry = GEOM_AUTOSIZE;
      break;

    case 0x26B76D5D: // "AutoSizeX"
      geometry = GEOM_AUTOSIZEX;
      break;

    case 0x227670EA: // "AutoSizeY"
      geometry = GEOM_AUTOSIZEY;
      break;

    case 0x5F178928: // "HInternal"
      geometry = GEOM_HINTERNAL;
      break;

    case 0xA09DA094: // "HExternal"
      geometry = GEOM_HINTERNAL;
      toggle = !toggle;
      break;

    case 0xEC9157FA: // "VInternal"
      geometry = GEOM_VINTERNAL;
      break;

    case 0x131B7E46: // "VExternal"
      geometry = GEOM_VINTERNAL;
      toggle = !toggle;
      break;

    case 0x2337C3D2: // "AlignToWidth"
      geometry = GEOM_ALIGNTOWIDTH;
      break;

    case 0xE9B11D12: // "AlignToHeight"
      geometry = GEOM_ALIGNTOHEIGHT;
      break;

    case 0x1432986D: // "KeepVisible"
      geometry = GEOM_KEEPVISIBLE;
      break;

    default:
      LOG_ERR(("Unknown Geometry setting [%s] for [%s]", s, Name()));
      return (FALSE);
  }

  // Toggle the geometry setting
  geom.flags = (toggle) ? (geom.flags | geometry) : (geom.flags & ~geometry);

  return TRUE;
}


//
// SetAlignTo
//
// Set the control to align to
//
Bool IControl::SetAlignTo(const char *name)
{
  alignTo = IFace::FindByName(name, this);
  return (alignTo.Alive());
}


//
// SetAlignTo
//
// Set the control to align to
//
Bool IControl::SetAlignTo(IControl *ctrl)
{
  alignTo = ctrl;
  return (alignTo.Alive());
}


//
// IControl::SetSize
//
void IControl::SetSize(S32 width, S32 height)
{
  // Set size
  size.x = width;
  size.y = height;
}


//
// IControl::SetGeomSize
//
void IControl::SetGeomSize(S32 width, S32 height)
{
  // Set the geometry size
  geom.size.x = width;
  geom.size.y = height;
}


//
// IControl::SetPos
//
void IControl::SetPos(S32 x, S32 y) 
{ 
  // Set position
  pos.x = x; 
  pos.y = y; 

  geom.pos.x = x;
  geom.pos.y = y;
}


//
// IControl::Set the color group
//
void IControl::SetColorGroup(ColorGroup *c)
{
  ASSERT(c)

  if (c)
  {
    paintInfo.colors = c;
  }
}


//
// IControl::OverrideTextColor
//
// Force the text color of this control
//
void IControl::OverrideTextColor(const Color *c)
{
  if (c)
  {
    forceTextClr = TRUE;
    forcedTextColor = *c;
  }
  else
  {
    forceTextClr = FALSE;
  }
}


//
// IControl::GetOverrideTextColor
//
// Get the forced text color, or NULL if not applicable
//
const Color *IControl::GetOverrideTextColor()
{
  return (forceTextClr ? &forcedTextColor : NULL);
}


//
// IControl::SetTextureSkin
//
void IControl::SetTextureSkin(TextureSkin *skinIn)
{
  skin = skinIn;
}


//
// IControl::SetImage
//
void IControl::SetImage(const TextureInfo *t) 
{ 
  if (t == NULL)
  {
    // Delete the texture info struct
    if (texture)
    {
      delete texture;
      texture = NULL;
      return;
    }
  }
  else
  {
    // Allocate a new texture info struct if needed
    if (texture == NULL)
    {
      texture = new TextureInfo;
    }

    // Copy info
    *texture = *t;
  }
}


//
// IControl::SetCursor
//
// Set the cursor from the cursor ID
//
void IControl::SetCursor(U32 id)
{
  cursor = id;
}


//
// IControl::SetVisible
//
// Set the visible (drawn) state
//
Bool IControl::SetVisible(Bool vis)
{
  Bool rc = controlState & STATE_VISIBLE ? TRUE : FALSE;

  if (vis)
  {
    controlState |= STATE_VISIBLE;
  }
  else
  {
    controlState &= ~STATE_VISIBLE;
  }

  // Return previous state
  return (rc);
}


//
// IControl::SetTipText
//
// Set tool tip text
//
void IControl::SetTipText(const CH *str, Bool dup)
{
  // Remove current string
  if (tipTextStr && freeTipText)
  {
    delete[] tipTextStr;
  }

  if (str)
  {
    if (dup)
    {
      tipTextStr = Utils::Strdup(str);
      freeTipText = TRUE;
    }
    else
    {
      tipTextStr = const_cast<CH *>(str);
      freeTipText = FALSE;
    }
  }
  else
  {
    // Clear the tip text
    tipTextStr = NULL;
    freeTipText = FALSE;
  }
}


//
// IControl::SetTextString
//
void IControl::SetTextString(const CH *str, Bool dup, Bool cleanup)
{ 
  ASSERT(str);

  // Remove current string
  if (textStr && freeText)
  {
    delete[] textStr;
  }

  if (dup)
  {
    textStr = Utils::Strdup(str);
    freeText = TRUE;
  }
  else
  {
    textStr = const_cast<CH *>(str);
    freeText = cleanup;
  }

  // Generate multi line setup
  if (controlStyle & STYLE_MULTILINE)
  {
    GenerateMultiLine(textStr);
  }
}


//
// IControl::GetTextString
//
// Get the text being displayed on this control
//
void IControl::GetTextString(CH *buf, U32 bufSize)
{
  Utils::Strmcpy(buf, GetTextString(), bufSize);
}


//
// IControl::GetTextString
//
// Get a pointer to the text being displayed on this control
//
const CH * IControl::GetTextString()
{
  if (textStr == NULL)
  {
//    LOG_WARN(("No text is defined for control '%s'", Name()))
    return (Utils::Ansi2Unicode(Name()));
  }
  else
  {
    return (textStr);
  }
}


//
// IControl::SetTextFont
//
void IControl::SetTextFont(const char *fontName)
{
  ASSERT(fontName);
  paintInfo.font = FontSys::GetFont(fontName);
}


//
// IControl::SetTextJustify
//
void IControl::SetTextJustify(U32 flag)
{
  textJustify = flag;
}


//
// IControl::SetFormatSpec
//
void IControl::SetFormatSpec(const char *format)
{
  ASSERT(format);

  // Delete current format specifier
  if (formatStr)
  {
    delete[] formatStr;
  }

  // Copy new format specifier
  formatStr = Utils::Strdup(format);
}


//
// IControl::AddEventScope
//
// Create a new event handler called "name"
//
void IControl::AddEventScope(const char *name, FScope *scope)
{
  U32 crc = Crc::CalcStr(name);

  // Ensure action does not exist
  if (actions.Find(crc))
  {
    ERR_FATAL(("Event Handler [%s] already defined for [%s]", name, Name()));
  }

  // Add it to the list
  actions.Add(crc, scope->Dup());
}


//
// IControl::ExecScope
//
// Execute the fscope
//
void IControl::ExecScope(FScope *fScope)
{
  fScope->InitIterators();

  // Step through each function in this scope
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    ExecItem(sScope);
  }
}


//
// IControl::ExecItem
//
// Execute a single item
//
void IControl::ExecItem(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x01E5156C: // "Activate"
    case 0x7619BFA6: // "Deactivate"
    case 0x5A377534: // "ToggleActive"
    case 0x40939940: // "Enable"
    case 0x80FFE965: // "Disable"
    case 0x31FBCA52: // "EnableRecurse"
    case 0x4988852B: // "DisableRecurse"
    case 0x74EF9BE3: // "Destroy"
    case 0x53EA6BFE: // "SetFocus"
    case 0xD921FC22: // "SetStyle"
    case 0x9FD16EF0: // "Resize"
    case 0x3381FB36: // "Move"
    case 0x5077882B: // "SetState"
    {
      IControl *ctrl = this;

      // Optional argument specifies the name of the control, using the
      // syntax defined in ICRoot::Find
      VNode *vNode = fScope->NextArgument(VNode::AT_STRING, FALSE);
      if (vNode)
      {
        // Use this control as the basis for finding the control
        const char *str;
        if (IFace::ExpandDerefVar(vNode->GetString(), TRUE, this, str))
        {
          ctrl = IFace::FindByName(str, this);
          if (!ctrl)
          {
            LOG_ERR(("%s: Control [%s] not found", fScope->NameStr(), str));
          }
        }
      }

      // Not allowed to do stuff to root
      if (ctrl == IFace::RootWindow())
      {
        LOG_ERR(("Tried to do bad stuff to root!"))
        //fScope->StackRecurse();
        return;
      }

      // If a control was found, perform operation on it
      if (ctrl)
      {
        switch (fScope->NameCrc())
        {
          case 0x01E5156C: // "Activate"
          {
            // 2nd argument specifies optional context
            VNode *vNode = fScope->NextArgument(VNode::AT_STRING, FALSE);
            if (vNode)
            {
              IControl *contextCtrl = NULL;
  
              // Use this control as the basis for finding the control
              const char *str;
              if (IFace::ExpandDerefVar(vNode->GetString(), TRUE, this, str))
              {
                contextCtrl = IFace::FindByName(str, this);
                if (!contextCtrl)
                {
                  LOG_ERR(("%s: Context Control [%s] not found", fScope->NameStr(), str));
                }
              }
              ctrl->SetContext(contextCtrl);
            }

            IFace::Activate(ctrl);
            break;
          }

          case 0x7619BFA6: // "Deactivate"
            IFace::Deactivate(ctrl);
            break;

          case 0x5A377534: // "ToggleActive"
            if (ctrl->ToggleActive() && ctrl->IsActive())
            {
              ctrl->SetZPos(0);
            }
            break;

          case 0x40939940: // "Enable"
            ctrl->SetEnabled(TRUE);
            break;

          case 0x80FFE965: // "Disable"
            ctrl->SetEnabled(FALSE);
            break;

          case 0x31FBCA52: // "EnableRecurse"
            ctrl->SetEnabled(TRUE, TRUE);
            break;

          case 0x4988852B: // "DisableRecurse"
            ctrl->SetEnabled(FALSE, TRUE);
            break;

          case 0x74EF9BE3: // "Destroy"
            ctrl->MarkForDeletion();
            break;

          case 0x53EA6BFE: // "SetFocus"
            ctrl->GetKeyFocus();
            break;

          case 0xD921FC22: // "SetStyle"
          {
            VNode *vNode;

            while ((vNode = fScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
            {
              const char *str = vNode->GetString();
              Bool toggle = TRUE;

              if (*str == '!')
              {
                toggle = FALSE;
                str++;
              }
              ctrl->SetStyleItem(str, toggle);
            }
            break;
          }

          case 0x9FD16EF0: // "Resize"
          {
            Bool active = ctrl->IsActive();

            S32 x = StdLoad::TypeU32(fScope);
            S32 y = StdLoad::TypeU32(fScope);

            // Deactivate first
            if (active)
            {
              ctrl->Deactivate();
            }

            // Resize
            ctrl->SetSize(x, y);
            ctrl->SetGeomSize(x, y);

            // Reactivate if necessary
            if (active)
            {
              ctrl->Activate();
            }
            break;
          }

          case 0x3381FB36: // "Move"
          {
            Bool active = ctrl->IsActive();

            S32 x = StdLoad::TypeU32(fScope);
            S32 y = StdLoad::TypeU32(fScope);

            // Deactivate first
            if (active)
            {
              ctrl->Deactivate();
            }

            // Move
            ctrl->SetPos(x, y);

            // Reactivate if necessary
            if (active)
            {
              ctrl->Activate();
            }
            break;
          }

          case 0x5077882B: // "SetState"
          {
            VNode *vNode;

            while ((vNode = fScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
            {
              const char *str = vNode->GetString();
              Bool toggle = TRUE;

              if (*str == '!')
              {
                toggle = FALSE;
                str++;
              }
              ctrl->SetStateItem(str, toggle);
            }
            break;
          }

        }
      }

      return;
    }

    case 0xECE0FB7B: // "SendNotifyEvent"
    case 0xF521756C: // "Notify"
    {
      // Send an event to a control
      const char *name = StdLoad::TypeString(fScope);
      const char *event = StdLoad::TypeString(fScope);
      StrCrc<64> param[3];
      param[0] = StdLoad::TypeStringD(fScope, "");
      param[1] = StdLoad::TypeStringD(fScope, "");
      param[2] = StdLoad::TypeStringD(fScope, "");

      for (int i = 0; i < 3; i++)
      {
        // Is the first character * ?
        /*
        Bool deref = FALSE;
        const char *untrans = param[i].str;
        if (*untrans == '*')
        {
          deref = TRUE;
          untrans++;
        }

        // Substitute variable name
        const char *str = FindVarName(untrans);

        // Do we want to derefence this var ?
        if (deref)
        {
          VarSys::VarItem *item = VarSys::FindVarItem(str);
          if (item)
          {
            str = item->GetStringValue();
          }
          else
          {
            LOG_WARN(("Cannot dereference var '%s'", str))
            return;
          }
        }
        */

        const char *str;
        if (IFace::ExpandDerefVar(param[i].str, TRUE, this, str))
        {
          param[i] = str;
        }
        else
        {
          return;
        }
      }

      IControl *ctrl;

      if ((ctrl = IFace::FindByName(name, this)) != NULL)
      {
        SendNotify(ctrl, Crc::CalcStr(event), FALSE, param[0].crc, param[1].crc, param[2].crc);
      }
      else
      {
        LOG_ERR(("Notify: could not find control [%s]", name))
      }
      return;
    }

    case 0xFF58B9E9: // "NotifyParent"
    {
      // Send a notification event to our parent
      SendNotify(parent, StdLoad::TypeStringCrc(fScope), FALSE);
      return;
    }

    case 0xC4FD8F50: // "Cmd"
    {
      // Execute a cosole command withing the scope of this control
      Console::ProcessCmd(StdLoad::TypeString(fScope), this);
      return;
    }

    case 0x698BE5EA: // "IFaceCmd"
    {
      // Execute a console command within the scope of this control
      ProcessCmd(fScope);
      return;
    }

    case 0x70DFC843: // "If"
    {
      if (Comparison::Function(fScope, this))
      {
        ExecScope(fScope);
      }
      else
      {
        if (fScope->ParentFunction())
        {
          FScope *nScope = fScope->ParentFunction()->PeekFunction();
          if (nScope && nScope->NameCrc() == 0x5F3F76C0) // "Else"
          {
            ExecScope(nScope);
          }
        }
      }
      break;
    }

    case 0x5F3F76C0: // "Else"
      // Ignore elses on their own
      break;


    case 0x4BED1273: // "Switch"
    {
      FSCOPE_DIRTY(fScope)

      // Get the name of the var to test on
      Switch::Value &value = Switch::GetValue(fScope, this);

      while (FScope *sScope = fScope->NextFunction())
      {
        if (sScope->NameCrc() == 0x97AF68BF) // "Case"
        {
          if (Switch::CompareValue(value, sScope, this))
          {
            ExecScope(sScope);
            break;
          }
        }
        else if (sScope->NameCrc() == 0x8F651465) // "Default"
        {
          ExecScope(sScope);
        }
      }

      Switch::DisposeValue(value);
      break;
    }

    case 0x2F0B8AF3: // "MessageBox"
    {
      // Get the message box title
      const char *title = StdLoad::TypeString(fScope, "Title");

      // Get the message box message
      const char *message = StdLoad::TypeString(fScope, "Message");

      // Get the control to send messages to
      const char *control = StdLoad::TypeString(fScope, "Control", "");
      //IControl *ctrl = *control ? IFace::FindByName(control, this) : NULL;
      IControl *ctrl = this;
      
      if (*control)
      {
        const char *str;
        if (IFace::ExpandDerefVar(control, TRUE, this, str))
        {
          ctrl = IFace::FindByName(str, this);
        }
      }

      // Message box events
      const char *text;
      const char *event;
      MBEvent *events[3];

      // The first button is required
      FScope *eScope = fScope->GetFunction("Button0");
      text = StdLoad::TypeString(eScope);
      if (eScope->IsNextArgString())
      {
        event = StdLoad::TypeString(eScope);
        events[0] = new MBEventNotify(text, TRANSLATE((text)), ctrl, Crc::CalcStr(event));
      }
      else
      {
        events[0] = new MBEvent(text, TRANSLATE((text)));
      }

      // The second and third buttons are optional
      eScope = fScope->GetFunction("Button1", FALSE);
      if (eScope)
      {
        text = StdLoad::TypeString(eScope);
        if (eScope->IsNextArgString())
        {
          event = StdLoad::TypeString(eScope);
          events[1] = new MBEventNotify(text, TRANSLATE((text)), ctrl, Crc::CalcStr(event));
        }
        else
        {
          events[1] = new MBEvent(text, TRANSLATE((text)));
        }
      }
      else
      {
        events[1] = NULL;
      }

      eScope = fScope->GetFunction("Button2", FALSE);
      if (eScope)
      {
        text = StdLoad::TypeString(eScope);
        if (eScope->IsNextArgString())
        {
          event = StdLoad::TypeString(eScope);
          events[2] = new MBEventNotify(text, TRANSLATE((text)), ctrl, Crc::CalcStr(event));
        }
        else
        {
          events[2] = new MBEvent(text, TRANSLATE((text)));
        }
      }
      else
      {
        events[2] = NULL;
      }

      IFace::MsgBox(TRANSLATE((title)), TRANSLATE((message)), 0, events[0], events[1], events[2]);
      return;
    }

    case 0xC39EE127: // "op"
    {
      Operation::Function(fScope, this);
      break;
    }

    default:
    {
      // Execute a console command at the global scope
      Main::ScopeHandler(fScope);
      return;
    }
  }
}


//
// IControl::AddEventTranslation
//
// Sets the destination control and event 'notify' to generate when the 
// actual event 'event' is handled.
//
Bool IControl::AddEventTranslation(U32 event, U32 notify, IControl *ctrl, U32 param1, U32 param2, U32 param3)
{
  ASSERT(ctrl)

  // Ensure it doesnt exist already
  if (translations.Find(event))
  {
    LOG_ERR(("Notification 0x%.8X for [%s] already exists", event, Name()))
    return (FALSE);
  }

  // Otherwise add it to the list
  translations.Add(event, new EventTranslation(notify, param1, param2, param3, ctrl));
  return (TRUE);
}


//
// IControl::SendNotify
//
// Sends a notification event to another control
//
Bool IControl::SendNotify(IControl *ctrl, U32 event, Bool translate, U32 param1, U32 param2, U32 param3)
{
  // Translate from a control generated system event to a user defined notification string
  if (translate)
  {
    EventTranslation *xlate;
 
    if ((xlate = translations.Find(event)) != NULL)
    {
      event = xlate->id;
      ctrl = xlate->ctrl;
      param1 = xlate->param1 == 0xFFFFFFFF ? param1 : xlate->param1;
      param2 = xlate->param2 == 0xFFFFFFFF ? param2 : xlate->param2;
      param3 = xlate->param3 == 0xFFFFFFFF ? param3 : xlate->param3;
    }
  }

  if (ctrl == NULL)
  {
    LOG_DIAG(("no ctrl"));
    return (FALSE);
  }

  // Send event directly to the control
  return (IFace::SendEvent(ctrl, this, IFace::NOTIFY, event, param1, param2, param3));
}


//
// IControl::NotifyAllChildren
//
// Sends a notification to all children
//
Bool IControl::NotifyAllChildren(U32 event, Bool translate, U32 param1, U32 param2, U32 param3, Bool propagate)
{
  for (NList<IControl>::Iterator i(&children); *i; i++)
  {
    SendNotify(*i, event, translate, param1, param2, param3);

    if (propagate)
    {
      (*i)->NotifyAllChildren(event, translate, param1, param2, param3, propagate);
    }
  }
  return (TRUE);
}


//
// IControl::AddToPollList
//
// Add this control to the poll list
//
void IControl::AddToPollList()
{
  ASSERT(!pollNode.InUse());
  pollList.Append(this);
}


//
// IControl::RemoveFromPollList
//
// Add a control to the poll list
//
void IControl::RemoveFromPollList()
{
  ASSERT(pollNode.InUse());
  pollList.Unlink(this);
}


//
// IControl::ProcessPollList
//
// Update all polled objects
//
void IControl::ProcessPollList(U32 timeStepMs)
{
  NList<IControl>::Node *node = pollList.GetHeadNode();

  while (node)
  {
    IControl *control = node->GetData();

    if ((control->nextPollTime -= timeStepMs) < 0)
    {
      // Do the processing
      control->Poll();

      // Set next poll time
      control->nextPollTime = control->pollInterval;
    }

    node = node->GetNext();
  }
}


//
// IControl::ProcessCmd
//
// Construct a console command using var substition within the 
// current control's var scope
//
void IControl::ProcessCmd(FScope *fScope)
{
  static char cmdBuf[256];

  cmdBuf[0] = '\0';
  VNode *node = fScope->NextArgument();

  while (node)
  {
    if (node->aType == VNode::AT_STRING)
    {
      // Is the first character * ?
      Bool deref = FALSE;
      const char *untrans = node->GetString();
      if (*untrans == '*')
      {
        deref = TRUE;
        untrans++;
      }

      // Substitute variable name
      const char *str = FindVarName(untrans);
      Bool quote = FALSE;

      // Do we want to derefence this var ?
      if (deref)
      {
        VarSys::VarItem *item = VarSys::FindVarItem(str);

        if (item)
        {
          str = item->GetStringValue();

          if (item->type == VarSys::VI_STRING)
          {
            quote = TRUE;
          }
        }
        else
        {
          LOG_WARN(("Cannot dereference var '%s'", str))
          return;
        }
      }

      // Copy the string in
      U32 len = strlen(cmdBuf);
      ASSERT(len < sizeof(cmdBuf));

      if (quote)
      {
        Utils::Sprintf(cmdBuf + len, sizeof(cmdBuf) - len - 1, "\"%s\" ", str);
      }
      else
      {
        Utils::Sprintf(cmdBuf + len, sizeof(cmdBuf) - len - 1, "%s ", str);
      }
    }
    else
    {
      LOG_ERR(("IFaceCmd: expecting string argument"));
      return;
    }

    node = fScope->NextArgument();
  }

  // Execute the command
  Console::ProcessCmd(cmdBuf);
}


///////////////////////////////////////////////////////////////////////////////
//
// Struct MultiLine - multi line text
//
IControl::MultiLine::MultiLine(U32 n) 
: count(n),
  items(NULL)
{
  if (count)
  {
    items = new Item[n];
  }
}

IControl::MultiLine::~MultiLine()
{
  if (items)
  {
    delete items;
  }
}
