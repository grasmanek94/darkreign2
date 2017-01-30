///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Heads up display
//
// 25-OCT-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "client_private.h"
#include "client_squadcontrol.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "promote.h"
#include "fontsys.h"
#include "common.h"
#include "transportobj.h"
#include "babel.h"
#include "multiplayer_data.h"
#include "weapon.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client
//
namespace Client
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace HUD
  //
  namespace HUD
  {
    // Scope name
    const char *SCOPE_NAME = "ConfigureHUD";

    // 90, 180, 270, 0 degrees
    static const F32 COS[4] = { 0.0F, -1.0F,  0.0F, 1.0F };
    static const F32 SIN[4] = { 1.0F,  0.0F, -1.0F, 0.0F };

    // Name of each corner function
    static const char *CORNER_STR[4] = { "Corner1", "Corner2", "Corner3", "Corner4" };

    // System initialised?
    static Bool sysInit = FALSE;


    //
    // Position information
    //
    struct PosInfo
    {
      enum
      {
        LEFT      = 0x0001,
        RIGHT     = 0x0002,
        TOP       = 0x0004,
        BOTTOM    = 0x0008,
        WIDTH     = 0x0010,
        HEIGHT    = 0x0020,
        HINTERNAL = 0x0040,
        VINTERNAL = 0x0080,
        HSCALE    = 0x0100,
        VSCALE    = 0x0200,
      };

      U32 flags;
      Point<S32> size;
      Point<S32> pos;
      Point<S32> scale;

      // Set scale from floating point values
      void SetScale(F32 x, F32 y)
      {
        scale.x = Utils::FtoL(x * 256.0F);
        scale.y = Utils::FtoL(y * 256.0F);
      }

      // Constructor
      PosInfo() : flags(0), size(0, 0), pos(0, 0)
      {
        SetScale(1.0F, 1.0F);
      }

      // Configure from an fscope
      void Read(FScope *fScope);

      // Adjust geometry
      void Adjust(ClipRect &rc, const ClipRect &relativeTo);
    };


    //
    // Definition of a reticle
    //
    struct Reticle
    {
      // Texture of each corner
      TextureInfo corner[4];

      // Color of corners
      Color color;

      // Minimum size, in pixels
      Point<S32> minSize;

      // Bar textures
      TextureInfo barBg;
      TextureInfo barFg;
      Color barColor;
      Point<S32> fgOfs0;
      Point<S32> fgOfs1;

      // Bar positions
      PosInfo healthBarPos;
      PosInfo taskBarPos;
      PosInfo ammoBarPos;
      PosInfo cargoBarPos;

      // Tree node (MUST BE LAST MEMBER!)
      NBinTree<Reticle>::Node node;


      // Constructor
      Reticle() : color(255L, 255L, 255L)
      {
        memset(this, 0, sizeof(Reticle) - sizeof node);
      }

      // Assignment operator
      Reticle &operator=(Reticle &rhs)
      {
        // Copy all data but the node
        memcpy(this, &rhs, sizeof(Reticle) - sizeof node);
        return (*this);
      }
    };


    //
    // Definition of a damage blip
    //
    struct DamageBlip
    {
      // Texture of each corner
      TextureInfo corner[4];

      // Color of texture
      Color color;

      // Tree node
      NBinTree<DamageBlip>::Node node;


      // Constructor
      DamageBlip() 
      {
        memset(this, 0, sizeof(DamageBlip) - sizeof node);
      }
    };


    //
    // Definition of a text item
    //
    struct TextItem
    {
      // Text
      Font *font;
      Color color;
      PosInfo pos;

      // Tree node
      NBinTree<TextItem>::Node node;

      // Constructor
      TextItem() : font(NULL), color(255L, 255L, 255L) {}

      // Render
      void Render(const CH *str, const ClipRect &screenRc, F32 alphaScale = 1.0F)
      {
        ASSERT(font)

        U32 len = Utils::Strlen(str);
        ClipRect rc(0, 0, font->Width(str, len) + 1, font->Height() + 1);
        pos.Adjust(rc, screenRc);
        font->Draw(rc.p0.x, rc.p0.y, str, len, color, &rc, alphaScale, 1);
      }
    };


    //
    // Reticle profile
    //
    struct ReticleProfile
    {
      Reticle *self;
      Reticle *ally;
      Reticle *neutral;
      Reticle *enemy;
      Reticle *selected;
      Reticle *teamMate;

      DamageBlip *damage;
      TextItem *squad;
      TextItem *unitName;
      TextItem *info;

      TextItem *id;
      TextItem *teamName;
      TextItem *task;

      NBinTree<ReticleProfile>::Node node;

      // Constructor
      ReticleProfile()
      {
        memset(this, 0, sizeof ReticleProfile);
      }
    };


    //
    // Status icon
    //
    struct StatusIcon
    {
      // Icon
      TextureInfo tex;
      Color color;

      // Position
      PosInfo pos;

      NBinTree<StatusIcon>::Node node;
    };


    // Is HUD display enabled
    static Bool enabled;

    // Registered color entries
    static BinTree<Color> colorEntries;

    // Registered reticles
    static NBinTree<Reticle> reticles(&Reticle::node);

    // Registered profiles
    static NBinTree<ReticleProfile> reticleProfiles(&ReticleProfile::node);

    // Registered damage blips
    static NBinTree<DamageBlip> damageBlips(&DamageBlip::node);

    // Registered text items
    static NBinTree<TextItem> textItems(&TextItem::node);

    // Status icons
    static NBinTree<StatusIcon> statusIcons(&StatusIcon::node);

    // Fade out (ms)
    static U32 fadeTime = 500;
    static F32 fadeTimeInv = 1.0F / F32(fadeTime);

    // Last moused over object
    static MapObjPtr mouseOver;

    // Time that mouse went over a unit
    static U32 mouseOverTime;

    // Mouse over hud alpha value
    static F32 overAlphaScale = 0.0F;

    // Text labels
    static TextItem *constructItem;
    static TextItem *playerMarker;

    // Forward declarations
    static void Configure(FScope *fScope);


    //
    // Initialise HUD system
    //
    void Init()
    {
      ASSERT(!sysInit)

      // Register the scope handler
      Main::RegisterRootScope(SCOPE_NAME, Configure);

      // Clear data
      constructItem = NULL;
      playerMarker = NULL;
      mouseOver = NULL;

      enabled = TRUE;
      sysInit = TRUE;
    }


    //
    // Shutdown HUD system
    //
    void Done()
    {
      ASSERT(sysInit)

      // Unregister the scope handler
      Main::UnregisterRootScope(SCOPE_NAME, Configure);

      // Delete resources
      colorEntries.DisposeAll();
      reticles.DisposeAll();
      reticleProfiles.DisposeAll();
      damageBlips.DisposeAll();
      textItems.DisposeAll();
      statusIcons.DisposeAll();

      sysInit = FALSE;
    }


    //
    // GetColorEntry
    //
    // Returns the given color entry, or a default color
    //
    Color GetColorEntry(U32 crc)
    {
      // Find the entry
      Color *c = colorEntries.Find(crc);

      // Return it, or the default
      return (c ? *c : Color(255L, 255L, 255L));
    }


    //
    // RenderStatusIcon
    //
    void RenderStatusIcon(U32 crc, const ClipRect &relative)
    {
      StatusIcon *icon = statusIcons.Find(crc);

      if (icon && icon->tex.texture)
      {
        // Calculate position relative to screen
        ClipRect rc(0, 0, 0, 0);
        icon->pos.Adjust(rc, relative);

        IFace::RenderRectangle(rc, icon->color, &icon->tex);
      }
    }


    //
    // DisplayUnit flags
    //
    enum
    {
      DU_SELECTED = (1 << 0),
      DU_TEAMMATE = (1 << 1),
      DU_RETICLE  = (1 << 2),
      DU_DAMAGE   = (1 << 4),
      DU_BARS     = (1 << 5),
      DU_SQUAD    = (1 << 6),
      DU_UNITNAME = (1 << 7),
      DU_DEBUG    = (1 << 8),
      DU_INFO     = (1 << 9),
    };


    //
    // Hook when the mouse over unit is changed
    //
    static void MouseOverChanged(MapObj *object)
    {
      mouseOver = object;

      if (object)
      {
        mouseOverTime = IFace::ElapsedMs();
        overAlphaScale = 0.0F;
      }
    }


    //
    // Render a bar
    //
    static void RenderBar(Reticle *reticle, ClipRect &bar, F32 percent, Color clr, F32 alphaScale, S32 min = 0)
    {
      // Render full background
      IFace::RenderRectangle(bar, reticle->barColor, &reticle->barBg, alphaScale);

      // Adjust coordinates for foreground
      bar.p0 += reticle->fgOfs0;
      bar.p1 += reticle->fgOfs1;

      // Reduce size of bar, but make it at least 2 pixels
      S32 width = Max<S32>(Utils::FtoL(F32(bar.Width()) * (percent + F32_MIN_MOD)), min);
      bar.p1.x = bar.p0.x + width;

      // Fix up UV coordinates of right hand edge
      TextureInfo tex = reticle->barFg;
      tex.uv.p1.x = tex.uv.p0.x + (tex.texture->InvWidth() * F32(width) / F32(bar.Width()));
      IFace::RenderRectangle(bar, clr, &tex, alphaScale);
    }


    //
    // Display a unit's HUD information
    //
    static void DisplayUnit(MapObj *mapObj, UnitObj *unitObj, U32 flags, F32 alphaScale, U32 age = 0)
    {
      ASSERT(mapObj)

      // Ignore this object if not on the display list
      if (!mapObj->displayNode.InUse())
      {
        return;
      }

#ifdef DEVELOPMENT
      if (!mapObj->Mesh().mesh)
      {
        ERR_FATAL(("HUD::DisplayUnit: %s has no root", mapObj->TypeName()));
      }
#endif

      // Project object's sphere into screen space
      Vector p;
      F32 rhw;
      Vid::ProjectFromWorld(p, mapObj->RootBounds().Offset(), rhw, mapObj->WorldMatrixRender());
      if (p.z < 0 || p.z > 1)
      {
        // Not visible
        return;
      }
      F32 rad = Vid::ProjectRHW( mapObj->RootBounds().Radius() * .8f, rhw);

      // Calculate top, left, bottom and right extents of sphere
      Vector dst[4];
      for (U32 i = 0; i < 4; i++)
      {
        dst[i].Set(p.x + COS[i] * rad, p.y + SIN[i] * rad, p.z);
      }

      ClipRect hudRect;
      hudRect.Set(Utils::FtoL(dst[1].x), Utils::FtoL(dst[0].y), Utils::FtoL(dst[3].x), Utils::FtoL(dst[2].y));
      hudRect.Sort();

      // Setup the reticle : FIXME
      //ReticleProfile *profile = reticleProfiles.Find(unitObj->UnitType()->GetReticleProfile());
      ReticleProfile *profile;
      
      if (mapObj->MapType()->GetFootPrintType())
      {
        profile = reticleProfiles.Find(0x858527B6); // "Large";
      }
      else
      {
        profile = reticleProfiles.Find(0xFCCB00D0); // "Small"
      }

      if (!profile)
      {
        return;
      }

      Reticle *reticle = NULL;

      // Work out the relation of this unit to the display team
      Relation relation = Team::GetRelation(Team::GetDisplayTeam(), unitObj ? unitObj->GetTeam() : NULL);

      // If unit is not on our team, use relationship to determine reticle
      if (unitObj && (unitObj->GetTeam() != Team::GetDisplayTeam()))
      {
        switch (relation)
        {
          case Relation::ENEMY:
            reticle = profile->enemy;
            break;

          case Relation::ALLY:
            reticle = profile->ally;
            break;

          case Relation::NEUTRAL:
            reticle = profile->neutral;
            break;
        }
      }
      else
      {
        // Unit is on our team, use mode to determine reticle
        reticle = (flags & DU_SELECTED) ? profile->selected : 
                  (flags & DU_TEAMMATE) ? profile->teamMate : profile->self;
      }

      // If no profile found, nothing to display
      if (!reticle)
      {
        return;
      }

      // Object health info
      F32 healthPct;
      Color healthColor;
      mapObj->GetHealthInfo(healthColor, healthPct);

      // Drop selection box out when it becomes too small
      if ((hudRect.Width() >= reticle->minSize.x) && (hudRect.Height() >= reticle->minSize.y))
      {
        // Render each corner
        Point<S32> ofs[4];

        if (flags & DU_RETICLE)
        {
          ofs[0].Set(hudRect.p0.x, hudRect.p0.y);
          ofs[1].Set(hudRect.p1.x - reticle->corner[1].pixels.Width(), hudRect.p0.y);
          ofs[2].Set(hudRect.p0.x, hudRect.p1.y - reticle->corner[2].pixels.Height());
          ofs[3].Set(hudRect.p1.x - reticle->corner[3].pixels.Width(), hudRect.p1.y - reticle->corner[3].pixels.Height());

          for (i = 0; i < 4; i++)
          {
            TextureInfo &ti = reticle->corner[i];

            if (ti.texture)
            {
              IFace::RenderRectangle(ClipRect(ofs[i], ofs[i] + ti.pixels.p1 - ti.pixels.p0), reticle->color, &ti, alphaScale);
            }
          }
        }

        if (flags & DU_DAMAGE && profile->damage)
        {
          DamageBlip *damage = profile->damage;

          ofs[0].Set(hudRect.p0.x, hudRect.p0.y);
          ofs[1].Set(hudRect.p1.x - damage->corner[1].pixels.Width(), hudRect.p0.y);
          ofs[2].Set(hudRect.p0.x, hudRect.p1.y - damage->corner[2].pixels.Height());
          ofs[3].Set(hudRect.p1.x - damage->corner[3].pixels.Width(), hudRect.p1.y - damage->corner[3].pixels.Height());

          // Render damage indicators
          for (i = 0; i < 4; i++)
          {
            TextureInfo &ti = profile->damage->corner[i];
            Color c = healthColor;

            // Fade out
            if (age > fadeTime)
            {
              break;
            }
            else
            {
              c.a = U8(Utils::FtoL(F32(c.a) * F32(fadeTime - age) * fadeTimeInv));
            }

            if (ti.texture)
            {
              IFace::RenderRectangle(ClipRect(ofs[i], ofs[i] + ti.pixels.p1 - ti.pixels.p0), c, &ti);
            }
          }
        }        

        // General information display
        if ((flags & DU_INFO) && profile->info && profile->info->font)
        {
          // Is this a unit
          if (unitObj)
          {
            // Should the commander of this unit be displayed
            Bool showCommander = FALSE;

            // Is this unit owned by the client team
            if (unitObj->GetTeam() == Team::GetDisplayTeam())
            {
              Task::RetrievedData taskInfo;

              // Give the task priority to display information
              if (unitObj->Retrieve(TaskRetrieve::Info, taskInfo))
              {
                profile->info->Render(taskInfo.s2, hudRect, alphaScale);
              }
              else

              // Is the object ready to upgrade
              if (unitObj->CanUpgradeNow())
              {
                if (UnitObjType *upgrade = unitObj->GetNextUpgrade())
                {
                  profile->info->Render(TRANSLATE(("#game.client.hud.upgrade", 1, upgrade->GetResourceCost())), hudRect, alphaScale);
                }
              }
              else

              // Is there an efficiency issue
              if (unitObj->GetEfficiency() < 0.98F)
              {
                profile->info->Render(TRANSLATE(("#game.client.hud.efficiency")), hudRect, alphaScale);
              }
              else
              {
                // Nothing else to show, so show the commander
                showCommander = TRUE;
              }
            }
            else
            {
              // Always show the commander of enemy units
              showCommander = TRUE;
            }

            // Do we need to display the commanders name
            if (showCommander && MultiPlayer::Data::Online())
            {
              if (Player *commander = Player::GetCommander(unitObj))
              {
                profile->info->Render(Utils::Ansi2Unicode(commander->GetName()), hudRect, alphaScale);
              }
            }
          }
          else

          // Is this a resource object
          if (ResourceObj *resource = Promote::Object<ResourceObjType, ResourceObj>(mapObj))
          {
            profile->info->Render(TRANSLATE(("#game.client.hud.resource", 1, resource->GetResource())), hudRect, alphaScale);
          }
        }

        // General bar display
        if ((flags & DU_BARS) && reticle->barBg.texture && reticle->barFg.texture)
        {
          // Render health bar
          if (mapObj->MapType()->GetHitPoints())
          {
            ClipRect bar = reticle->barBg.pixels;
            reticle->healthBarPos.Adjust(bar, hudRect);
            RenderBar(reticle, bar, healthPct, healthColor, alphaScale, 2);
          }

          // Display information only available to allies
          if (relation == Relation::ALLY)
          {
            // See if there is a secondary bar to draw
            Task::RetrievedData taskInfo;

            // Ask the task if it has progress to display
            if (mapObj->Retrieve(TaskRetrieve::Progress, taskInfo))
            {
              ClipRect bar = reticle->barBg.pixels;
              reticle->taskBarPos.Adjust(bar, hudRect);
              RenderBar(reticle, bar, taskInfo.f1, GetColorEntry(taskInfo.u1), alphaScale);
            }

            // Unit specific information
            if (unitObj)
            {
              // Does this unit have a weapon
              if (unitObj->GetWeapon())
              {
                ClipRect bar = reticle->barBg.pixels;
                reticle->ammoBarPos.Adjust(bar, hudRect);

                // Is there ammo to display
                if (unitObj->GetMaximumAmmunition())
                {
                  // "Unit::Ammunition"
                  RenderBar
                  (
                    reticle, bar, 
                    F32(unitObj->GetAmmunition()) / F32(unitObj->GetMaximumAmmunition()), 
                    GetColorEntry(0x96E5AD55), alphaScale
                  );
                }
                else
                {
                  // Get the fire delay from the weapon type
                  F32 delay = unitObj->GetWeapon()->GetType().GetDelayIM().GetFPoint();

                  // Is it long enough to display
                  if (delay > 10.0F)
                  {
                    // "Unit::Ammunition"
                    RenderBar(reticle, bar, unitObj->GetWeapon()->GetDelayPercent(), GetColorEntry(0x96E5AD55), alphaScale);
                  }
                }
              }

              // Is this a transporter
              if (TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(unitObj))
              {
                if (transport->TransportType()->GetSpaces())
                {
                  // The number of pixels between each bar
                  const S32 gap = 2;

                  // The minimum size of each bar
                  const S32 min = 3;

                  ClipRect bar = reticle->barBg.pixels;
                  reticle->cargoBarPos.Adjust(bar, hudRect);

                  // Get the cargo information
                  U32 capacity = transport->TransportType()->GetSpaces();
                  U32 cargo = transport->GetUsedSpaces();

                  // Work out the size of each box
                  S32 box = Max<S32>(min, bar.Width() / capacity);

                  // Work out starting position for the boxes
                  S32 start = bar.MidX() - (((capacity * (box + gap)) - gap) / 2);

                  Color colorF = GetColorEntry(0x26E5BB54); // "Transport::Cargo::Full"
                  Color colorE = GetColorEntry(0x4F49D9DA); // "Transport::Cargo::Empty"

                  // Draw each box
                  for (U32 i = 0; i < capacity; i++)
                  {
                    Point<S32> p0(start + (i * (box + gap)), bar.p0.y);
                    ClipRect r(p0, p0.x + box, bar.p1.y);
                    RenderBar(reticle, r, 1.0F, (i < cargo) ? colorF : colorE, alphaScale);
                  }
                }
              }
            }
          }
        }

        // Unit specific display
        if (unitObj)
        {
          // Display information to players on this units team
          if (unitObj->GetTeam() == Team::GetDisplayTeam())
          {
            SquadObj *squad = unitObj->GetSquad();

            // Squad id
            if ((flags & DU_SQUAD) && squad && profile->squad && profile->squad->font)
            {
              U32 clientId;

              if (Client::SquadControl::MapSquadToClient(squad->Id(), clientId))
              {
                CH buf[33];
                profile->squad->Render(Utils::ItoA(clientId, buf, 10), hudRect, alphaScale);
              }
            }
          }

          // Type name
          if ((flags & DU_UNITNAME) && profile->unitName && profile->unitName->font)
          {
            profile->unitName->Render(unitObj->GetUpgradedUnit().GetDesc(), hudRect, alphaScale);
          }
        }

        // Development
        if ((flags & DU_DEBUG) && Common::Debug::data.hud)
        {
          Team *team = unitObj ? unitObj->GetTeam() : NULL;

          // Id
          if (profile->id && profile->id->font)
          {
            CH buf[32];
            Utils::Sprintf(buf, 32, L"o%d s%d", mapObj->Id(), unitObj ? (unitObj->GetSquad() ? unitObj->GetSquad()->Id() : 0) : 0);
            profile->id->Render(buf, hudRect, alphaScale);
          }

          // Team Name
          if (profile->teamName && profile->teamName->font)
          {
            profile->teamName->Render(Utils::Ansi2Unicode(team ? team->GetName() : "[None]"), hudRect, alphaScale);
          }

          // Task
          Task *task = mapObj->GetCurrentTask();

          if (task && profile->task && profile->task->font)
          {
            char buf[64];
            Utils::Sprintf(buf, 64, "%s %s %.0f", task->GetName(), task->Info(), GameTime::TimeSinceCycle(task->GetInvoked()));
            profile->task->Render(Utils::Ansi2Unicode(buf), hudRect, alphaScale);
          }
        }
      }
    }


    //
    // Render the HUD
    //
    void Render()
    {
      if (!enabled)
      {
        return;
      }

      PERF_S(("HUD"))

      ASSERT(Team::GetDisplayTeam())

      // Default flags for selected units
      U32 flags = DU_RETICLE | DU_BARS | DU_SELECTED | DU_DEBUG;

      if (data.fullSquadHud)
      {
        flags |= DU_SQUAD;
      }

      // If only one unit selected, also show information
      if (data.sList.GetCount() == 1)
      {
        flags |= DU_INFO;
      }

      // Selected objects
      for (UnitObjList::Iterator i(&data.sList); *i; i++)
      {
        if (UnitObj *u = (*i)->GetPointer())
        {
          DisplayUnit(u, u, flags, IFace::data.alphaScale);
        }
      }

      // Moused over object
      if (data.cInfo.gameWnd.Alive() && data.cInfo.o.map.Alive())
      {
        MapObj *object = data.cInfo.o.map;

        if
        (
          // Is this a unit
          (
            data.cInfo.o.unit.Alive() && 
            data.cInfo.o.unit->UnitType()->IsSelectable() && 
            data.cInfo.o.unit->TestHaveSeen(Team::GetDisplayTeam()->GetId())
          )
          
          || 

          // Is this a resource
          Promote::Type<ResourceObjType>(object->GameType())
        )
        {
          // Trigger mouse over changed hook
          if (mouseOver.Dead() || (mouseOver->Id() != object->Id()))
          {
            MouseOverChanged(object);
          }

          // Animate alpha
          if (overAlphaScale < 1.0F)
          {
            // Fade to full alpha over 0.3 seconds
            overAlphaScale = Min<F32>(overAlphaScale + F32(IFace::TimeStepMs()) * 0.003F, 1.0F);
          }

          // Moused over object
          DisplayUnit
          (
            object, Promote::Object<UnitObjType, UnitObj>(object), 
            DU_RETICLE | DU_BARS | DU_SQUAD | DU_UNITNAME | DU_DEBUG | DU_INFO, 
            overAlphaScale * IFace::data.alphaScale
          );
        }
      }
      else
      {
        MouseOverChanged(NULL);
      }

      // Units selected by team mates
      if (Player::GetCurrentPlayer() && Team::GetDisplayTeam())
      {
        for (int j = 0; j < Game::MAX_PLAYERS; j++)
        {
          Player *player = Player::Id2Player(j);

          // Do we have a player who is not the client's player whose team is the same as the client's player ?
          if (player && (player != Player::GetCurrentPlayer()) && (player->GetTeam() == Team::GetDisplayTeam() && (player->GetType() != Player::AI)))
          {
            // Draw the objects this player has selected
            for (i.SetList(&player->GetSelectedList()); *i; i++)
            {
              if (UnitObj *u = (*i)->GetPointer())
              {
                DisplayUnit(u, u, DU_RETICLE | DU_TEAMMATE, IFace::data.alphaScale, 0);
              }
            }
          }
        }
      }

      // Ouch list
      MapObjOuchList &list = MapObjCtrl::GetOuchList();
      MapObjOuchList::Iterator ouch(&list);
      MapObjOuchListNode *node;

      while ((node = ouch++) != NULL)
      {
        if (node->Alive())
        {
          if (node->age < 1000)
          {
            if (UnitObj *u = Promote::Object<UnitObjType, UnitObj>(*node))
            {
              DisplayUnit(u, u, DU_DAMAGE, IFace::data.alphaScale, node->age);
            }

            node->age += Main::elapTime;
          }
          else
          {
            list.Dispose(node);
          }
        }
        else
        {
          list.Dispose(node);
        }
      }

      // Construction object
      if 
      (
        // Game window is alive
        data.cInfo.gameWnd.Alive() && 
        
        // In a construction mode
        (data.cInfo.pEvent == PE_CONSTRUCT || data.cInfo.pEvent == PE_NOCONSTRUCT) && 
        
        // The text item is configured
        constructItem && constructItem->font
      )
      {
        if (data.constructType.Alive())
        {
          constructItem->Render(data.constructType->GetDesc(), ClipRect(data.cInfo.mouse, data.cInfo.mouse));
        }
      }

      // Player markers - note metre pos is set up in RenderPlayerMarkers
      if (playerMarker && playerMarker->font)
      {
        for (NBinTree<Display::PlayerMarker>::Iterator marker(&Display::markers); *marker; marker++)
        {
          Display::PlayerMarker *pm = *marker;

          // Filter out ally but allied markers
          if (pm->canSee)
          {
            Vector p;
            F32 rhw;

            // Project ground point into screen space
            if (pm->pos.z > Vid::Math::nearPlane && pm->pos.z < Vid::Math::farPlane)
		        {
              Vid::ProjectFromWorld( p, pm->pos, rhw);

              // Display on screen
              Point<S32> pt(Utils::FastFtoL(p.x), Utils::FastFtoL(p.y));
              playerMarker->Render(pm->player->GetDisplayName(), ClipRect(pt, pt));
            }
          }
        }
      }

      PERF_E(("HUD"))
    }


    //
    // Enable or disable the HUG
    //
    void Enable(Bool f)
    {
      if (enabled != f)
      {
        // Prevent a barrage of flashes when the HUD is re-enabled
        if (f)
        {
          MapObjCtrl::GetOuchList().Clear();
        }
      }
      enabled = f;
    }


    //
    // ConfigureColorEntries
    //
    // Load the 
    //
    static void ConfigureColorEntries(FScope *fScope)
    {
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x9F1D54D0: // "Add"
          {
            // Get the entry identifier
            const char *ident = StdLoad::TypeString(sScope);

            // And the crc
            U32 crc = Crc::CalcStr(ident);

            // Ignore if already defined
            if (!colorEntries.Find(crc))
            {
              // Create the new color
              Color *c = new Color;

              // Load from the config
              StdLoad::TypeColor(sScope, *c);

              // Add it to the tree
              colorEntries.Add(crc, c);
            }
            else
            {
              LOG_ERR(("Color entry [%s] already defined", ident))
            }
          }
        }
      }
    }


    //
    // ConfigureReticle
    //
    static void ConfigureReticle(FScope *fScope)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *templt = StdLoad::TypeStringD(fScope, "");
      U32 nameCrc = Crc::CalcStr(name);

      if (reticles.Find(nameCrc))
      {
        LOG_ERR(("Reticle [%s] already defined", name))
        return;
      }

      // Find the optional template
      Reticle *r = new Reticle;

      if (*templt)
      {
        U32 templtCrc = Crc::CalcStr(templt);
        Reticle *ret;

        if ((ret = reticles.Find(templtCrc)) != NULL)
        {
          *r = *ret;
        }
        else
        {
          LOG_ERR(("Templete Reticle [%s] not found", templt))
        }
      }

      // Load corner definitions
      for (U32 i = 0; i < 4; i++)
      {
        FScope *sScope;
        
        if ((sScope = fScope->GetFunction(CORNER_STR[i], FALSE)) != NULL)
        {
          IFace::FScopeToTextureInfo(sScope, r->corner[i]);
        }
      }

      // Load color
      StdLoad::TypeColor(fScope, "Color", r->color);

      // Minimum size
      StdLoad::TypePoint(fScope, "MinSize", r->minSize, r->minSize);

      // Load bar definitions
      FScope *sScope;

      if ((sScope = fScope->GetFunction("Bar", FALSE)) != NULL)
      {
        StdLoad::TypeColor(sScope, "Color", r->barColor, r->barColor);
        IFace::FScopeToTextureInfo(sScope->GetFunction("Background"), r->barBg);
        IFace::FScopeToTextureInfo(sScope->GetFunction("Foreground"), r->barFg);
        StdLoad::TypePoint(sScope, "FgOffsetTL", r->fgOfs0, r->fgOfs0);
        StdLoad::TypePoint(sScope, "FgOffsetBR", r->fgOfs1, r->fgOfs1);
      }

      // Bar positions
      if ((sScope = fScope->GetFunction("HealthBarPosition", FALSE)) != NULL)
      {
        r->healthBarPos.Read(sScope);
      }
      if ((sScope = fScope->GetFunction("TaskBarPosition", FALSE)) != NULL)
      {
        r->taskBarPos.Read(sScope);
      }
      if ((sScope = fScope->GetFunction("AmmoBarPosition", FALSE)) != NULL)
      {
        r->ammoBarPos.Read(sScope);
      }
      if ((sScope = fScope->GetFunction("CargoBarPosition", FALSE)) != NULL)
      {
        r->cargoBarPos.Read(sScope);
      }

      // Add it to the list
      reticles.Add(nameCrc, r);
    }


    //
    // CreateReticleProfile
    //
    static void CreateReticleProfile(FScope *fScope)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 nameCrc = Crc::CalcStr(name);

      if (reticleProfiles.Find(nameCrc))
      {
        LOG_ERR(("ReticleProfile [%s] already defined", name));
        return;
      }

      ReticleProfile *r = new ReticleProfile;

      // Setup each item
      r->self     = reticles.Find(StdLoad::TypeStringCrc(fScope, "Self"));
      r->selected = reticles.Find(StdLoad::TypeStringCrc(fScope, "Selected"));
      r->ally     = reticles.Find(StdLoad::TypeStringCrc(fScope, "Ally"));
      r->neutral  = reticles.Find(StdLoad::TypeStringCrc(fScope, "Neutral"));
      r->enemy    = reticles.Find(StdLoad::TypeStringCrc(fScope, "Enemy"));
      r->teamMate = reticles.Find(StdLoad::TypeStringCrc(fScope, "TeamMate"));

      r->damage   = damageBlips.Find(StdLoad::TypeStringCrc(fScope, "DamageBlip"));
      r->squad    = textItems.Find(StdLoad::TypeStringCrc(fScope, "SquadInfo"));
      r->unitName = textItems.Find(StdLoad::TypeStringCrc(fScope, "UnitName"));
      r->info     = textItems.Find(StdLoad::TypeStringCrc(fScope, "Info"));

      r->id       = textItems.Find(StdLoad::TypeStringCrc(fScope, "Id"));
      r->teamName = textItems.Find(StdLoad::TypeStringCrc(fScope, "TeamName"));
      r->task     = textItems.Find(StdLoad::TypeStringCrc(fScope, "Task"));

      // Add it to the list
      reticleProfiles.Add(nameCrc, r);
    }


    //
    // ConfigureDamageBlip
    //
    static void ConfigureDamageBlip(FScope *fScope)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 nameCrc = Crc::CalcStr(name);

      if (damageBlips.Find(nameCrc))
      {
        LOG_ERR(("DamageBlip [%s] already defined", name))
      }

      DamageBlip *b = new DamageBlip;

      // Load corner definitions
      for (U32 i = 0; i < 4; i++)
      {
        FScope *sScope;
        
        if ((sScope = fScope->GetFunction(CORNER_STR[i], FALSE)) != NULL)
        {
          IFace::FScopeToTextureInfo(sScope, b->corner[i]);
        }
      }

      // Load color
      StdLoad::TypeColor(fScope, "Color", b->color);

      // Add it to the list
      damageBlips.Add(nameCrc, b);
    }


    //
    // ConfigureTextItem
    //
    static void ConfigureTextItem(FScope *fScope)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 nameCrc = Crc::CalcStr(name);
      FScope *sScope;

      if (textItems.Find(nameCrc))
      {
        LOG_ERR(("TextItem [%s] already defined", name))
      }

      TextItem *p = new TextItem;

      // Load font
      p->font = FontSys::GetFont(StdLoad::TypeString(fScope, "Font"));

      // Load color
      StdLoad::TypeColor(fScope, "Color", p->color, p->color);

      // Load alignment
      if ((sScope = fScope->GetFunction("Position", FALSE)) != NULL)
      {
        p->pos.Read(sScope);
      }

      // Add it to the list
      textItems.Add(nameCrc, p);
    }


    //
    // ConfigureStatusIcon
    //
    static void ConfigureStatusIcons(FScope *fScope)
    {
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x9F1D54D0: // "Add"
          {
            const char *name = StdLoad::TypeString(sScope);
            U32 id = Crc::CalcStr(name);

            if (statusIcons.Find(id))
            {
              LOG_ERR(("StatusIcon %s already defined", name))
            }
            else
            {
              StatusIcon *newIcon = new StatusIcon;

              IFace::FScopeToTextureInfo(sScope->GetFunction("Image"), newIcon->tex);
              StdLoad::TypeColor(sScope, "Color", newIcon->color);
              newIcon->pos.Read(sScope->GetFunction("Pos"));

              statusIcons.Add(id, newIcon);
            }
            break;
          }
        }
      }
    }


    //
    // Configure
    //
    static void Configure(FScope *fScope)
    {
      FScope *sScope;

      if (!sysInit)
      {
        LOG_ERR(("ConfigureHUD:  HUD system not initialised yet"))
        return;
      }

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x34F52409: // "ConfigureColorEntries"
          {
            ConfigureColorEntries(sScope);
            break;
          }

          case 0x7A6B4921: // "ConfigureReticle"
          {
            ConfigureReticle(sScope);
            break;         
          }

          case 0x514A3B8E: // "CreateReticleProfile"
          {
            CreateReticleProfile(sScope);
            break;
          }

          case 0xEBA257D6: // "ConfigureDamageBlip"
          {
            ConfigureDamageBlip(sScope);
            break;
          }

          case 0x1D70928A: // "ConfigureTextItem"
          {
            ConfigureTextItem(sScope);
            break;
          }

          case 0x1C80BC9B: // "ConfigureConstruction"
          {
            constructItem = textItems.Find(StdLoad::TypeStringCrc(sScope));
            break;
          }

          case 0x09E9F145: // "ConfigurePlayerMarker"
          {
            playerMarker = textItems.Find(StdLoad::TypeStringCrc(sScope));
            break;
          }

          case 0x346CD541: // "FadeTime"
          {
            fadeTime = StdLoad::TypeU32(sScope, 500, Range<U32>(100, 2000));
            fadeTimeInv = 1.0F / F32(fadeTime);
            break;
          }

          case 0x6AD93521: // "SelectionSkin"
          {
            Display::selectionSkin = IFace::FindTextureSkin(StdLoad::TypeStringCrc(sScope));
            break;
          }

          case 0xB47EC0AF: // "ConfigureStatusIcons"
          {
            ConfigureStatusIcons(sScope);
            break;
          }
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Struct PosInfo
    //

    //
    // PosInfo::Read
    //
    void PosInfo::Read(FScope *fScope) 
    {
      FScope *sScope;

      // Read size
      StdLoad::TypePoint(fScope, "Size", size, size);

      // Read pos
      StdLoad::TypePoint(fScope, "Pos", pos, pos);

      // Read scale, convert to fixed point
      Point<F32> f(1.0F, 1.0F);

      StdLoad::TypePoint(fScope, "Scale", f, f);
      SetScale(f.x, f.y);

      // Read geometry
      if ((sScope = fScope->GetFunction("Align", FALSE)) != NULL)
      {
        VNode *vNode;

        while ((vNode = sScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
        {
          const char *str = vNode->GetString();

          switch (Crc::CalcStr(str))
          {
            case 0xBA190163: // "Left"
              flags |= LEFT;
              break;

            case 0xE2DDD72B: // "Right"
              flags |= RIGHT;
              break;

            case 0x239B3316: // "Top"
              flags |= TOP;
              break;

            case 0x5270B6BD: // "Bottom"
              flags |= BOTTOM;
              break;

            case 0x2F6D7F50: // "Width"
              flags |= WIDTH;
              break;

            case 0x86010476: // "Height"
              flags |= HEIGHT;
              break;

            case 0x5F178928: // "HInternal"
              flags |= HINTERNAL;
              break;

            case 0xEC9157FA: // "VInternal"
              flags |= VINTERNAL;
              break;

            case 0xFBEAFA4C: // "HScale"
              flags |= HSCALE;
              break;

            case 0xCE1B74AB: // "VScale"
              flags |= VSCALE;
              break;
          }
        }
      }
    }


    //
    // PosInfo::Adjust
    //
    void PosInfo::Adjust(ClipRect &rc, const ClipRect &relativeTo) 
    {
      Point<S32> newSize = rc.p1 - rc.p0;
      Point<S32> newPos;

      // Size
      if (flags & WIDTH)
      {
        newSize.x = relativeTo.Width();
      }

      if (flags & HEIGHT)
      {
        newSize.y = relativeTo.Height();
      }

      // Apply absolute size
      newSize += size;

      // Apply relative scale
      if (flags & HSCALE)
      {
        newSize.x = (scale.x * newSize.x) >> 8;
      }
      if (flags & VSCALE)
      {
        newSize.y = (scale.y * newSize.y) >> 8;
      }

      // Horizontal
      if (flags & LEFT)
      {
        newPos.x = -(flags & HINTERNAL ? 0 : newSize.x) + pos.x;
      }
      else
      if (flags & RIGHT)
      {
        newPos.x = relativeTo.Width() + pos.x - (flags & HINTERNAL ? newSize.x : 0);
      }
      else
      {
        newPos.x = (relativeTo.Width() - newSize.x) / 2 + pos.x;
      }
      newPos.x += relativeTo.p0.x;

      // Vertical
      if (flags & TOP)
      {
        newPos.y = -(flags & VINTERNAL ? 0 : newSize.y) + pos.y;
      }
      else
      if (flags & BOTTOM)
      {
        newPos.y = relativeTo.Height() + pos.y - (flags & VINTERNAL ? newSize.y : 0);
      }
      else
      {
        newPos.y = (relativeTo.Height() - newSize.y) / 2 + pos.y;
      }
      newPos.y += relativeTo.p0.y;

      // Fill out return struct
      rc.p0 = newPos;
      rc.p1 = newPos + newSize;
    }
  }
}
