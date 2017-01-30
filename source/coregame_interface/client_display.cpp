///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Events
//
// 19-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "client_private.h"
#include "regionobj.h"
#include "iface.h"
#include "unitobj.h"
#include "meshent.h"
#include "terrain.h"
#include "viewer.h"
#include "promote.h"
#include "footprint.h"
#include "sight.h"
#include "sound.h"
#include "mapobjouchlist.h"
#include "common.h"
#include "iface_util.h"
#include "wallobj.h"
#include "particle.h"
#include "particlesystem.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client
//
namespace Client
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display
  //
  namespace Display
  {

    // Multiplayer markers
    NBinTree<PlayerMarker> markers(&PlayerMarker::node);

    // Selection box skin
    TextureSkin *selectionSkin;

    // FootPrint placement display
    static Common::Display::PlacementDeform placeDeform;

    // Id of unit currently displaying a rally point
    static U32 rallyUnitId;

    // Rally point runner particle
    static ParticlePtr rallyParticle;

    // Rally point location
    static Point<U32> rallyPoint;


    //
    // RenderSpecialized
    //
    // Display specialized unit information
    //
    static void RenderSpecialized()
    {
      // Is there a unit under the cursor
      UnitObj *over = data.cInfo.gameWnd.Alive() ? data.cInfo.o.unit.GetPointer() : NULL;

      // Can we see this units special info
      if (over && Team::TestDisplayRelation(over, Relation::ALLY))
      {
        over->RenderSpecialized();
      }

      // Now display for all selected units
      for (UnitObjList::Iterator i(&data.sList); *i; ++i)
      {
        if (UnitObj *u = (*i)->GetPointer())
        {
          if (u != over)
          {
            u->RenderSpecialized();
          }
        }
      }
    }


    //
    // RenderRallyPoint
    //
    // Display a rally point
    //
    static void RenderRallyPoint()
    {
      Point<S32> srcPoint(0, 0);
      Point<U32> dstPoint(0, 0);
      Matrix src, dst;

      // Get the single selected unit
      UnitObj *unit = data.cInfo.oneUnit.GetPointer();

      // Is there a rally point
      if (unit && unit->GetFootInstance() && unit->GetRallyPoint(dstPoint))
      {
        // Setup destination
        dst.ClearData();
        dst.posit.x = WorldCtrl::CellToMetresX(dstPoint.x);
        dst.posit.z = WorldCtrl::CellToMetresZ(dstPoint.z);
        dst.posit.y = TerrainData::FindFloorWithWater(dst.posit.x, dst.posit.z);

        // Find the source
        srcPoint.Set(S32(dstPoint.x), S32(dstPoint.z));
        unit->GetFootInstance()->ClampToFringe(srcPoint);
        src.ClearData();
        src.posit.x = WorldCtrl::CellToMetresX(srcPoint.x);
        src.posit.z = WorldCtrl::CellToMetresZ(srcPoint.z);
        src.posit.y = TerrainData::FindFloorWithWater(src.posit.x, src.posit.z);
      }
      else
      {
        // Act as if not selected
        unit = NULL;
      }

      if 
      (
        // Rally unit has been deselected
        (!unit && rallyUnitId) 
        
        || 
        
        (
          // Have a rally point unit
          unit 
          
          && 
          
          (
            // Different to last one
            (unit->Id() != rallyUnitId) 
            
            || 
            
            // Rally point location has changed
            (rallyUnitId && (dstPoint != rallyPoint))
          )
        )
      )
      {
        // Dispose of any current particle
        if (rallyParticle.Alive())
        {
          delete rallyParticle;
        }

        // Is there a unit selected
        if (unit)
        {
          // Never display a trail when same point
          if ((srcPoint.x != S32(dstPoint.x)) || (srcPoint.z != S32(dstPoint.z)))
          {
            // Create the runner particle
            if (ParticleClass *p = ParticleSystem::FindType(0x94E362BD)) // "Client::Rally"
            {
              Matrix m(src);
              m.posit.y += 3.0F;
			        Vector v(0.0f, 0.0f, 0.0f);
              rallyParticle = ParticleSystem::New(p, m, v, v, dst.posit - src.posit, 0.0F);
            }
          }

          // Remember this unit's info
          rallyUnitId = unit->Id();
          rallyPoint = dstPoint;
        }
        else
        {
          rallyUnitId = 0;
        }
      }

      // Render the start and end points
      if (unit)
      {     
        Common::Display::Mesh(0x693D5359, src, Color(0.0F, 0.8F, 0.0F, 0.7F)); // "Location"
        Common::Display::Mesh(0x693D5359, dst, Color(0.0F, 1.0F, 0.0F, 0.7F)); // "Location"
      }
    }


    //
    // RenderCurrentRegion
    //
    // Display currently selected region
    //
    static void RenderCurrentRegion()
    {
      if (data.paintRegion.Alive())
      {
        Common::Display::MarkAreaMetre(data.paintRegion->GetArea());
      } 
    }


    //
    // RenderFootPrint
    //
    // Render an object placement footprint
    //
    static void RenderFootPrint(Bool deform)
    {
      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Are we placing a footprint
      if (i.gameWnd.Alive() && (i.pEvent == PE_CONSTRUCT || i.pEvent == PE_NOCONSTRUCT))
      {
        ASSERT(data.constructType.Alive())
        ASSERT(i.t.cell)

        // Does object have a footprint
        FootPrint::Type *foot = data.constructType->GetFootPrintType();

        // Equality check because type was changed on console during dev
        if (foot && (foot == &data.placeFoot.GetType()))
        {
          if (deform)
          {
            Common::Display::FootPrintPlacementDeform(data.placeFoot, placeDeform, TRUE, &data.placeLocation, data.clientAlpha.GetValue());
          }
          else
          {
            Common::Display::FootPrintPlacement(data.placeFoot, TRUE, &data.placeLocation, data.clientAlpha.GetValue());
          }
        }

        if (i.pEvent == PE_CONSTRUCT)
        {
          // Is this type a wall
          if (WallObjType *wallType = Promote::Type<WallObjType>(data.constructType))
          {
            wallType->DisplayAvailableLinks(i.t.pos);
          }
        }
      }
    }


    //
    // RenderFormation
    //
    // Render the formation mesh
    //
    static void RenderFormation()
    {
      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Are we placing a formation
      if (i.gameWnd.Alive() && (i.pEvent == PE_FORMATION))
      {
        Vector pos;
        pos.x = data.cInfo.t.pos.x;
        pos.z = data.cInfo.t.pos.z;
        pos.y = TerrainData::FindFloor(pos.x, pos.z) + 2.0f;
        Matrix m;
        WorldCtrl::SetupWorldMatrix(m, pos, data.dir);

        Common::Display::Mesh(0x7223612A, m); // , Color(1.0f, 1.0f, 1.0f)); // "Formation"
      }
    }


    //
    // RenderSelectionBox
    //
    static void RenderSelectionBox(const Area<S32> &rect)
    {
      if (selectionSkin)
      {
        // Setup paintinfo
        PaintInfo pi(selectionSkin->states[ColorGroup::NORMAL].colors);
        pi.client = pi.window = rect;

        selectionSkin->Render(pi, ColorGroup::NORMAL);
      }
      else
      {
        IFace::RenderRectangle(rect, Common::Display::GroupSelectColor());
      }
    }


    //
    // RenderPlayerMarkers
    //
    static void RenderPlayerMarkers()
    {
      Matrix m = Matrix::I;

      for (NBinTree<PlayerMarker>::Iterator i(&markers); *i; i++)
      {
        PlayerMarker *p = *i;

        // allowed to see it
        if (p->canSee)
        {
          m.posit.x = WorldCtrl::CellToMetresX(p->cx);
          m.posit.z = WorldCtrl::CellToMetresZ(p->cz);
          m.posit.y = TerrainData::FindFloorWithWater(m.posit.x, m.posit.z);

          p->pos = m.posit;

          // "Location"
          Common::Display::Mesh(0x693D5359, m, p->player->GetTeam()->GetColor());
        }
      }
    }


    //
    // PreRender
    //
    // Pre-render processing
    //
    void PreRender()
    {
      // Update the current camera
      Viewer::Simulate();
      Viewer::Render(0xF530E366); // "Render::PreRender"

      /*
      // Get the camera matrix
      const Matrix &m = Vid::CurCamera().WorldMatrix();

      // Update the 3D listener position
      Sound::Digital::UpdateListener
      (
        m.front.x, m.front.y, m.front.z, m.up.x, m.up.y, m.up.z
      );
      */

      // Update pulsating alpha
      data.clientAlpha.Simulate(Main::ElapTimeFrame());
    }


    //
    // PreTerrain
    //
    // Rendering before terrain is drawn
    //
    void PreTerrain()
    {
      // Render an object placement footprint
      if (Common::Debug::data.footDeform)
      {
        RenderFootPrint(TRUE);
      }
    }


    //
    // Render
    //
    // Client specific rendering
    //
    void Render()
    {
      // Display a region
      RenderCurrentRegion();

      // Render an object placement footprint
      if (!Common::Debug::data.footDeform)
      {
        RenderFootPrint(FALSE);
      }

      // Render specialized unit information
      RenderSpecialized();

      // Display a rally point
      RenderRallyPoint();

      // Render formation placement
      RenderFormation();

      // Render trails
      Trail::Render();

      // Render player markers
      RenderPlayerMarkers();

      // Viewer rendering
      Viewer::Render(0xEF30A860); // "Render::PostObject"

      // Display debug information
      #ifdef DEVELOPMENT

        Debug::Render();

      #endif

      // Render interface
      if (!IFace::GetFlag(IFace::DISABLE_DRAW))
      {
        IFace::InitRender();

        // Display the current group selection box on top of main game window
        if (data.selectMode == SM_PREDRAW)
        {
          RenderSelectionBox(data.mouseRect);
        }

        // Render HUD
        HUD::Render();
        
        // Render main portion of interface
        IFace::CallRenderProc();

        // Render any other systems that require iface
        Viewer::Render(0xCEEF613F); // "Render::PostIFace1"
        Viewer::Render(0xC3AC47E6); // "Render::PostIFace2"

        // Display the current group selection box on top of map window
        if (data.selectMode == SM_POSTDRAW)
        {
          RenderSelectionBox(data.mouseRect);
        }

        // Display status icons last
        if (!Viewer::Action::IsPlaying())
        {
          ClipRect screenRect = IFace::RootWindow()->GetPaintInfo().window;

          if (GameTime::IsFallingBehind())
          {
            Client::HUD::RenderStatusIcon(0x267B119C, screenRect); // "FallingBehind"
          }

          if (GameTime::IsDroppingFrames())
          {
            Client::HUD::RenderStatusIcon(0xF144EDC2, screenRect); // "DroppingFrames"
          }

          if (GameTime::IsStalled())
          {
            Client::HUD::RenderStatusIcon(0x540B86C8, screenRect); // "NetworkLag"
          }
        }

        IFace::DoneRender();
      }
    }


    //
    // PostRender
    //
    // Called after rendering is finished
    //
    void PostRender()
    {
      Common::Display::PostFootPrintPlacementDeform(placeDeform);
    }

  
    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      // Clear the region reaper
      data.paintRegion = NULL;

      // Clear local data
      placeDeform.Init();
      rallyUnitId = 0;
      rallyParticle = NULL;
      selectionSkin = NULL;
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      selectionSkin = NULL;
      markers.DisposeAll();
    }


    //
    // SetPlayerMarker
    //
    void SetPlayerMarker(const Player *player, S32 cx, S32 cz)
    {
      ASSERT(player)
      ASSERT(WorldCtrl::CellOnMap(cx, cz))

      PlayerMarker *marker;

      if ((marker = markers.Find(player->GetId())) == NULL)
      {
        marker = new PlayerMarker;
        marker->player = player;
        marker->canSee = Team::TestRelation(Team::GetDisplayTeam(), player->GetTeam(), Relation::ALLY) ? TRUE : FALSE;

        markers.Add(player->GetId(), marker);
      }

      marker->cx = cx;
      marker->cz = cz;
    }


    //
    // ClearPlayerMarker
    //
    void ClearPlayerMarker(const Player *player)
    {
      PlayerMarker *marker;

      if ((marker = markers.Find(player->GetId())) != NULL)
      {
        markers.Dispose(marker);
      }
    }
  }
}
