///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 4-DEC-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_prereqlist.h"
#include "common.h"
#include "iface.h"
#include "iface_util.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PrereqList
  //

  //
  // Constructor
  //
  PrereqList::PrereqList(IControl *parent) 
  : IControl(parent),
    team(NULL),
    type(NULL)
  {
    // Default control style
    controlStyle |= STYLE_FADEIN;
  }


  //
  // Destructor
  //
  PrereqList::~PrereqList()
  {
    if (preview.Alive())
    {
      preview->MarkForDeletion();
    }
  }


  //
  // Setup
  //
  // Configure control from an FScope
  //
  void PrereqList::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x4BB73281: // "CostKey"
        costKey = StdLoad::TypeString(fScope);
        break;

      case 0x44FC1D1A: // "TimeKey"
        timeKey = StdLoad::TypeString(fScope);
        break;

      // Pass it to the previous level in the hierarchy
      default:
        IControl::Setup(fScope);
    }
  }


  //
  // Called after Configure() is completed
  //
  void PrereqList::PostConfigure()
  {
    IControl::PostConfigure();

    type->InitializeResources();
    if (!type->IsNullObj() && fullPreview)
    {
      preview = new TypePreview(IFace::OverlaysWindow(), type, "Move");
    }
  }


  //
  // Activate
  //
  Bool PrereqList::Activate()
  {
    if (IControl::Activate())
    {
      if (preview.Alive())
      {
        preview->Activate();
        preview->Deactivate();
      }
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Setup
  //
  void PrereqList::Setup(const Point<S32> &pointIn, Team *teamIn, UnitObjType *typeIn, UnitObj *constructorIn)
  {
    point = pointIn;
    team = teamIn;
    type = typeIn;
    constructor = constructorIn;
  }


  //
  // Draw control
  //
  void PrereqList::DrawSelf(PaintInfo &pi)
  {
    if (constructor.Alive())
    {
      // Draw all the items
      S32 height = pi.client.Height();
      S32 yoffset = height / 2;
      S32 x = point.x;
      S32 y = point.y;

      // If we're on the left hand side of the screen, we're on the left
      Bool left = point.x < IFace::RootWindow()->GetSize().x / 2;
      
      if (left)
      {
        // Slight hack to move to the right hand side of the icon 
        x += 41;
      }
      else
      {
        // Move the client/window areas up to the top left
        Point<S32> offset(pi.client.p1.x, pi.client.p0.y);

        pi.client -= offset;
        pi.window -= offset;
      }

      x += left ? 30 : -30;
      Point<S32> o(x, y);

      pi.client += o;
      pi.window += o;

      // Display the description
      if (preview.Alive())
      {
        PaintInfo p = pi;

        S32 y = p.client.p0.y;
        S32 maxy = IFace::RootWindow()->GetSize().y - 49 - p.client.Width();
        if (y > maxy)
        {
          y = maxy;
        }

        p.window.p0.y = y + p.window.p0.y - p.client.p0.y;
        p.client.p0.y = y;
        p.window.p1.y = p.window.p0.y + p.window.Width();
        p.client.p1.y = p.client.p0.y + p.client.Width();
        preview->DrawSelf(p);

        // Draw connecting line
        ClipRect connect(x + (left ? -30 : 0), point.y + yoffset, x + (left ? 0 : 30), point.y + yoffset + 1);
        IFace::RenderRectangle(connect, Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        x += left ? pi.client.Width() : -pi.client.Width();
      }
      else
      {
        controlState |= STATE_HILITE;
        DrawCtrlBackground(pi, GetTexture());
        DrawCtrlFrame(pi);
        DrawCtrlText(pi, type->GetDesc());
        controlState &= ~STATE_HILITE;

        // Draw connecting line
        ClipRect connect(x + (left ? -30 : 0), point.y + yoffset, x + (left ? 0 : 30), point.y + yoffset + 1);
        IFace::RenderRectangle(connect, Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        x += left ? pi.client.Width() : -pi.client.Width();

        ClipRect savedClient = pi.client;
        ClipRect savedWindow = pi.window;

        Point<S32> costOffset(0, pi.client.Height() + 3);
        pi.client += costOffset;
        pi.window += costOffset;
        //pi.client.p0.x += 40;
        //pi.window.p0.x += 40;

        controlState |= STATE_SELECTED;
        DrawCtrlBackground(pi, GetTexture());
        DrawCtrlFrame(pi);

        // Does this type cost resource
        if (type->GetResourceCost())
        {
          DrawCtrlText(pi, TRANSLATE((costKey.str, 1, type->GetResourceCost())));
        }
        else
        {
          CH buf[128];
          F32 v = type->GetConstructionTime();
          Utils::Sprintf(buf, 128, L"%u:%02u", (U32)(v / 60.0F), (U32)fmod(v, 60.0F));
          DrawCtrlText(pi, TRANSLATE((timeKey.str, 1, buf)));
        }
        controlState &= ~STATE_SELECTED;

        // Draw connecting lines
        connect.p0.x += left ? 25 : 5;
        connect.p1.x = connect.p0.x + 1;
        connect.p1.y = connect.p0.y + yoffset * 2 + 5;
        IFace::RenderRectangle(connect, Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        if (left)
        {
          connect.p1.x += 4;
        }
        else
        {
          connect.p0.x -= 4;
        }
        connect.p0.y = connect.p1.y - 1;
        IFace::RenderRectangle(connect, Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);

        // Restore previous positions
        pi.client = savedClient;
        pi.window = savedWindow;
      }

      pi.client -= o;
      pi.window -= o;

      // Build up the list of prereqs which we don't currently have
      BinTree<UnitObjType> types;
      List<UnitObjType> list;
      Build(TRUE, type, types, list);
      
      // Make sure that we don't get too close to the bottom of the screen
      S32 maxy = IFace::RootWindow()->GetSize().y - 49 - types.GetCount() * (height + 3);
      if (y > maxy)
      {
        y = maxy;
      }

      S32 bottom = y;
      S32 savey = y;

      x += left ? 30 : -30;

      // Iterate the types and and them
      for (BinTree<UnitObjType>::Iterator t(&types); *t; t++)
      {
        UnitObjType *type = *t;

        pi.client += Point<S32>(x, y);
        pi.window += Point<S32>(x, y);

        if (t.GetKey() > 0)
        {
          controlState |= STATE_DISABLED;
        }

        DrawCtrlBackground(pi, GetTexture());
        DrawCtrlFrame(pi);
        DrawCtrlText(pi, type->GetDesc());

        if (t.GetKey() > 0)
        {
          controlState &= ~STATE_DISABLED;
        }

        S32 height = pi.client.Height();
        S32 yoffset = height / 2;

        // Draw horizontal line
        IFace::RenderRectangle(ClipRect(x + (left ? -5 : 0), y + yoffset, x + (left ? 0 : 5), y + 1 + yoffset), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);

        pi.client -= Point<S32>(x, y);
        pi.window -= Point<S32>(x, y);

        bottom = y;

        y += height + 3;

      }

      x += left ? -30 : 30;

      types.UnlinkAll();

      // Draw lines
      if (y > savey)
      {
        IFace::RenderRectangle(ClipRect(x + (left ? 0 : -25), point.y + yoffset, x + (left ? 25 : 0), point.y + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        IFace::RenderRectangle(ClipRect(x + (left ? 24 : -25), savey + yoffset, x + (left ? 25 : -24), bottom + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
      }
    }
    else
    {
      MarkForDeletion();
    }
  }


  //
  // Build
  //
  U32 PrereqList::Build(Bool first, UnitObjType *type, BinTree<UnitObjType> &types, List<UnitObjType> &list)
  {
    U32 count = 0;

    // Add this type to the list
    list.Append(type);

    // Get the prereqs
    for (UnitObjTypeList::Iterator t(&type->GetPrereqs().Get()); *t; t++)
    {
      UnitObjType *type = **t;

      // Only add it if we ...
      // Don't Have It
      if 
      (
        !Prereq::HaveType(team, type) ||
        (first && constructor->MissingUpgrade(type))
      )
      {
        Bool found = FALSE;

        // See if the type is in our list
        for (List<UnitObjType>::Iterator i(&list); *i; i++)
        {
          if (*i == type)
          {
            found = TRUE;
            break;
          }
        }

        if (found)
        {
          continue;
        }

        found = FALSE;

        count++;

        // Check to see if this type is already in the tree
        for (BinTree<UnitObjType>::Iterator t(&types); *t; t++)
        {
          if (*t == type)
          {
            found = TRUE;
            break;
          }
        }

        U32 num = Build(FALSE, type, types, list);

        count += num;

        if (!found)
        {
          types.Add(num, type);
        }
      }
    }

    list.Unlink(type);

    return (count);
  }

}
