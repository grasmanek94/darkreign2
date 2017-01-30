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
#include "common_prereqtree.h"
#include "iface.h"
#include "iface_util.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PrereqTree
  //

  //
  // Constructor
  //
  PrereqTree::PrereqTree(IControl *parent) 
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
  PrereqTree::~PrereqTree()
  {
  }


  //
  // Setup
  //
  void PrereqTree::Setup(const Point<S32> &pointIn, Team *teamIn, UnitObjType *typeIn, UnitObj *constructorIn)
  {
    point = pointIn;
    team = teamIn;
    type = typeIn;
    constructor = constructorIn;
  }


  //
  // Draw control
  //
  void PrereqTree::DrawSelf(PaintInfo &pi)
  {
    if (constructor.Alive())
    {
      // Draw all the items
      S32 height = pi.client.Height();
      S32 yoffset = height / 2;
      S32 x = point.x;
      S32 y = point.y;

      // Move the client/window areas up to the top left
      Point<S32> offset(pi.client.p1.x, pi.client.p0.y);

      pi.client -= offset;
      pi.window -= offset;

      x -= 30;
      Point<S32> o(x, y);

      pi.client += o;
      pi.window += o;

      controlState |= STATE_HILITE;
      DrawCtrlBackground(pi, GetTexture());
      DrawCtrlFrame(pi);
      DrawCtrlText(pi, type->GetDesc());
      controlState &= ~STATE_HILITE;

      pi.client -= o;
      pi.window -= o;

      IFace::RenderRectangle(ClipRect(point.x - 30, point.y + yoffset, point.x, point.y + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
      x -= pi.client.Width();

      S32 savey = y;
      S32 bottom = savey;
      Draw(TRUE, pi, x - 30, y, bottom, type);

      // Draw lines
      if (y > savey)
      {
        IFace::RenderRectangle(ClipRect(x - 25, point.y + yoffset, x, point.y + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        IFace::RenderRectangle(ClipRect(x - 25, savey + yoffset, x - 24, bottom + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
      }
    }
    else
    {
      MarkForDeletion();
    }
  }


  //
  // Draw 
  //
  void PrereqTree::Draw(Bool first, PaintInfo &pi, S32 x, S32 &y, S32 &bottom, UnitObjType *type)
  {
    // Get the prereqs
    for (UnitObjTypeList::Iterator t(&type->GetPrereqs().Get()); *t; t++)
    {
      UnitObjType *type = **t;

      // Only add it if we ...
      // Don't Have It
      if (
        !Prereq::HaveType(team, type) ||
        (first && constructor->MissingUpgrade(type)))
      {
        pi.client += Point<S32>(x, y);
        pi.window += Point<S32>(x, y);

        DrawCtrlBackground(pi, GetTexture());
        DrawCtrlFrame(pi);
        DrawCtrlText(pi, type->GetDesc());

        S32 height = pi.client.Height();
        S32 yoffset = height / 2;

        // Draw horizontal line
        IFace::RenderRectangle(ClipRect(x, y + yoffset, x + 5, y + 1 + yoffset), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);

        pi.client -= Point<S32>(x, y);
        pi.window -= Point<S32>(x, y);

        bottom = y;

        y += height + 5;
        S32 savey = y;
        S32 bottom = savey;

        // Draw children
        Draw(FALSE, pi, x - 10, y, bottom, type);

        // Draw vertical line (if neccesary)
        if (y > savey)
        {
          IFace::RenderRectangle(ClipRect(x - 5, savey - 5, x - 4, bottom + yoffset + 1), Color(1.0f, 1.0f, 1.0f, pi.alphaScale), NULL, pi.alphaScale);
        }
      }
    }
  }

}
