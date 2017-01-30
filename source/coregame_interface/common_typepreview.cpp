/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 15-MAR-2000
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_typepreview.h"
#include "iface_util.h"
#include "font.h"
#include "promote.h"
#include "babel.h"
#include "weapon.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TypePreview
  //



  //
  // TypePreview::TypePreview
  //
  TypePreview::TypePreview(IControl *parent)
  : ICMesh(parent)
  {
    // Default control style
    controlStyle |= STYLE_FADEIN;

    ReadTemplate("Common::TypePreview");
  }


  //
  // TypePreview::TypePreview
  //
  TypePreview::TypePreview(IControl *parent, MapObjType *type, const char *anim)
  : ICMesh(parent),
    type(type)
  {
    // Default control style
    controlStyle |= STYLE_FADEIN;

    SetMesh(type->GetMeshRoot(), anim);

    sunVector.x = 0.5;
    sunVector.y = -1.0;
    sunVector.z = 0.5;
    sunVector.Normalize();

    ReadTemplate("Common::TypePreview");
  }


  //
  // TypePreview::~TypePreview
  //
  TypePreview::~TypePreview()
  {
  }


  //
  // TypePreview::DrawSelf
  //
  // Draw the button control
  //
  void TypePreview::DrawSelf(PaintInfo &pi)
  {
    //DrawCtrlBackground(pi);

    IFace::RenderRectangle
    (
      pi.client, 
      Color(0.0f, 0.0f, 0.0f, 1.0f), // ->bg[ColorIndex()], 
      NULL,
      pi.alphaScale
    );

    ICMesh::DrawSelf(pi);
    ICMesh::DrawCtrlFrame(pi);

    if (pi.font)
    {
      const CH *text;
      U32 length;
      U32 height = pi.font->Height() + 5;
      S32 y = 5;

      text = type->GetDesc();
      length = Utils::Strlen(text);

      pi.font->Draw
      (
        pi.client.p0.x + 5, 
        pi.client.p0.y + y, 
        text, 
        length,
        Color(1.0f, 1.0f, 1.0f, 1.0f),
        &pi.client,
        pi.alphaScale
      );
      y += height;

      // Is this a unit
      UnitObjType *unitType = Promote::Type<UnitObjType>(type);

      if (unitType)
      {
        // Add the cost, if there is one
        if (unitType->GetResourceCost())
        {
          text = TRANSLATE(("#game.client.prereqlist.cost", 1, unitType->GetResourceCost()));
          length = Utils::Strlen(text);

          y = pi.client.Height() - height;
          pi.font->Draw
          (
            pi.client.p0.x + 5, 
            pi.client.p0.y + y, 
            text, 
            length,
            Color(1.0f, 1.0f, 1.0f, 1.0f),
            &pi.client,
            pi.alphaScale
          );
        }

        // Add the construction time
        {
          CH buf[128];
          F32 v = unitType->GetConstructionTime();
          Utils::Sprintf(buf, 128, L"%u:%02u", (U32)(v / 60.0F), (U32)fmod(v, 60.0F));
          text = TRANSLATE(("#game.client.prereqlist.time", 1, buf));
          length = Utils::Strlen(text);

          y = pi.client.Height() - height;
          pi.font->Draw
          (
            pi.client.p1.x - pi.font->Width(text, length) - 5, 
            pi.client.p0.y + y, 
            text, 
            length,
            Color(1.0f, 1.0f, 1.0f, 1.0f),
            &pi.client,
            pi.alphaScale
          );
        }

        // Add the hit points
        if (unitType->GetHitPoints() + unitType->GetArmour())
        {
          text = TRANSLATE(("#game.client.prereqlist.hp", 1, unitType->GetHitPoints() + unitType->GetArmour()));
          length = Utils::Strlen(text);

          y = pi.client.Height() - height - height;
          pi.font->Draw
          (
            pi.client.p1.x - pi.font->Width(text, length) - 5, 
            pi.client.p0.y + y, 
            text, 
            length,
            Color(1.0f, 1.0f, 1.0f, 1.0f),
            &pi.client,
            pi.alphaScale
          );
        }

        // Add the command points
        if (unitType->GetCommandCost())
        {
          text = TRANSLATE(("#game.client.prereqlist.cp", 1, unitType->GetCommandCost()));
          length = Utils::Strlen(text);

          y = pi.client.Height() - height - height - height;
          pi.font->Draw
          (
            pi.client.p1.x - pi.font->Width(text, length) - 5, 
            pi.client.p0.y + y, 
            text, 
            length,
            Color(1.0f, 1.0f, 1.0f, 1.0f),
            &pi.client,
            pi.alphaScale
          );
        }

        // Does this unit type bear a weapon ?
        Weapon::Type *weapon = unitType->GetWeapon();

        if (weapon)
        {
          // Infantry
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x + 6,
              pi.client.p1.y - height - 5 - 
              ((ArmourClass::Lookup(
                weapon->GetDamage().GetDamageId(), 
                ArmourClass::Crc2ArmourClassId(0x236B9FE8) // "armor_infantry"
              ) * 40) >> 16),
              pi.client.p0.x + 11,
              pi.client.p1.y - height - 5
            ),
            Color(1.0f, 0.0f, 0.0f, 0.5f), 
            NULL, 
            pi.alphaScale
          );

          // Vehicle
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x + 12,
              pi.client.p1.y - height - 5 - 
              ((ArmourClass::Lookup(
                weapon->GetDamage().GetDamageId(), 
                ArmourClass::Crc2ArmourClassId(0x27244F22) // "armor_vehicle"
              ) * 40) >> 16),
              pi.client.p0.x + 17, 
              pi.client.p1.y - height - 5
            ),
            Color(0.0f, 1.0f, 0.0f, 0.5f), 
            NULL, 
            pi.alphaScale
          );

          // Structure
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x + 18,
              pi.client.p1.y - height - 5 - 
              ((ArmourClass::Lookup(
                weapon->GetDamage().GetDamageId(), 
                ArmourClass::Crc2ArmourClassId(0x9C0F0A88) // "armor_structure"
              ) * 40) >> 16),
              pi.client.p0.x + 23, 
              pi.client.p1.y - height - 5
            ),
            Color(1.0f, 1.0f, 0.0f, 0.5f), 
            NULL, 
            pi.alphaScale
          );

          // Flyer
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x + 24,
              pi.client.p1.y - height - 5 - 
              ((ArmourClass::Lookup(
                weapon->GetDamage().GetDamageId(), 
                ArmourClass::Crc2ArmourClassId(0x0C7486D1) // "armor_flyer"
              ) * 40) >> 16),
              pi.client.p0.x + 29, 
              pi.client.p1.y - height - 5
            ),
            Color(0.0f, 1.0f, 1.0f, 0.5f), 
            NULL, 
            pi.alphaScale
          );

          // Naval
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x + 30,
              pi.client.p1.y - height - 5 - 
              ((ArmourClass::Lookup(
                weapon->GetDamage().GetDamageId(), 
                ArmourClass::Crc2ArmourClassId(0x2A9AB611) // "armor_naval"
              ) * 40) >> 16),
              pi.client.p0.x + 35, 
              pi.client.p1.y - height - 5
            ),
            Color(0.0f, 0.0f, 1.0f, 0.5f), 
            NULL, 
            pi.alphaScale
          );

          IFace::RenderLine
          (
            Point<S32>(pi.client.p0.x + 4, pi.client.p1.y - height - 5),
            Point<S32>(pi.client.p0.x + 37, pi.client.p1.y - height - 5),
            Color(1.0f, 1.0f, 1.0f, 0.5f), pi.alphaScale
          );

        }
      }
    }
  }
}