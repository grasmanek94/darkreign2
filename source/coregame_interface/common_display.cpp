///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"
#include "terrain_priv.h"

#include "common.h"
#include "mapobj.h"
#include "main.h"
#include "console.h"
#include "meshent.h"
#include "mapobjctrl.h"
#include "physicsctrl.h"
#include "environment.h"
#include "promote.h"
#include "explosionobj.h"
#include "render.h"
#include "claim.h"
#include "team.h"
#include "particlesystem.h"
#include "particle.h"

#include "transportobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Common - Code used in both the game and the studio
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display - Various common display functions
  //
  namespace Display
  {
    VarInteger clear;
    VarInteger bright;

    // System initialized flag
    static Bool initialized = FALSE;
    static Bool initializedSim;

    // Meshes
    static BinTree<MeshRoot> meshes;

    // Textures
    static BinTree<Bitmap> textures;

    // Colors - these could be a list but its a bit wasteful with a color being 4 bytes
    static Color colorGroupSelect(0L, 128L, 0L, 88L);

    // Struct Marker
    struct Marker
    {
      F32 value;
      NBinTree<Marker>::Node node;

      Marker(F32 value)
      : value(value)
      {
      }

    };

    // Markers
    static NBinTree<Marker> markers(&Marker::node);

    // The particle used for rendering trail segments
    static ParticlePtr trailParticle;


    //
    // Prototypes
    //
    static void RenderTrailSegment(const Vector &p0, const Vector &p1);


    //
    // GroupSelectColor
    //
    // Return the group selection color
    //
    Color GroupSelectColor()
    {
      return (colorGroupSelect);
    }


    //
    // GetTexture
    //
    // Get a texture asset
    //
    Bitmap *GetTexture(U32 id)
    {
      ASSERT(initialized);
      return (textures.Find(id));
    }


    //
    // Rectangle
    //
    // Draws a rectangle flat on the screen
    //
    void Rectangle(Color color, const Area<S32> &rect)
    {
      ASSERT(initialized);

      // Draw the group selection box
      Vid::SetBucketPrimitiveDesc
      (
        PT_TRIANGLELIST, FVF_TLVERTEX, 
        DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF
      );

      Vid::SetBucketMaterial(Vid::defMaterial);
      Vid::SetBucketTexture(NULL, color.a < 255 ? TRUE : FALSE);
      Vid::SetTranBucketZ(0.0f);

      VertexTL *vertmem;
      U16 *indexmem;

      if (Vid::LockIndexedPrimitiveMem((void **)&vertmem, 4, &indexmem, 6))
      {
        vertmem[0].vv.x = (F32) rect.p0.x;
        vertmem[0].vv.y = (F32) rect.p0.y;
        vertmem[1].vv.x = (F32) rect.p1.x;
        vertmem[1].vv.y = (F32) rect.p0.y;
        vertmem[2].vv.x = (F32) rect.p1.x;
        vertmem[2].vv.y = (F32) rect.p1.y;
        vertmem[3].vv.x = (F32) rect.p0.x;
        vertmem[3].vv.y = (F32) rect.p1.y;

        // Assign a color to each vertex
        vertmem[0].diffuse = color;
        vertmem[1].diffuse = color;
        vertmem[2].diffuse = color;
        vertmem[3].diffuse = color;

        for (U32 i = 0; i < 4; i++)
        {
          vertmem[i].vv.z = 0.0f;
          vertmem[i].rhw  = 1.0f;
          vertmem[i].specular = 0xff000000;
        }

        Utils::Memcpy( indexmem, Vid::rectIndices, 12);

        Vid::UnlockIndexedPrimitiveMem(4, 6);
      }
    }


    //
    // ObjectGroundSprite
    //
    // Displays the given texture on the ground below the object
    //
    void ObjectGroundSprite(MapObj *obj, Color color, Bitmap *texture)
    {
      ASSERT(initialized)

      // Should we use default texture
      if (!texture)
      {
        texture = textures.Find(0x1483BCF7); // "HealthBar"
      }

      TerrainData::RenderGroundSpriteWithWater
      (
        obj->WorldMatrixRender().Position(), obj->RootBounds().Radius(),
        obj->WorldMatrixRender().Front(),
        texture, color, RS_BLEND_DEF,
        UVPair(0,0), UVPair(1,0), UVPair(1,1)
      );   
    }


    //
    // ObjectSelection
    //
    // Displays an object as selected
    //
    void ObjectSelection(MapObj *obj, Bitmap *texture, U32 alpha)
    {
      ASSERT(initialized);

      U32 green;

      // Should we use default texture
      if (!texture)
      {
        texture = textures.Find(0x1483BCF7); // "HealthBar";
      }

      // Calculate the green portion
      if (obj->MapType()->GetHitPoints())
      {
        green = U32
        (
          255.0F * F32(Clamp<S32>(0, obj->GetHitPoints(), obj->MapType()->GetHitPoints())) / 
          F32(obj->MapType()->GetHitPoints())
        );
      }
      else
      {
        green = 255;
      }

      // Display the object
      ObjectGroundSprite(obj, Color((U32) 255 - green, green, (U32) 0, alpha), texture);

      // If this is a linked object draw a path to the object to which it is linked
      TransportObj *transportObj = Promote::Object<TransportObjType, TransportObj>(obj);
      if (transportObj)
      {
        TransportObj *link = transportObj->GetTelepadLink();
        if (link)
        {
          RenderTrailSegment(transportObj->Origin(), link->Origin());
        }
      }
    }


    //
    // ObjectHealthBar
    //
    // Draws the health of a map object
    //
    void ObjectHealthBar(MapObj *obj, Bitmap *tex, U32 alpha)
    {
      // Don't render for objects with no max hitpoints
      if (obj->MapType()->GetHitPoints())
      {
        // Get the health color
        Color c;
        F32 pct;
        
        obj->GetHealthInfo(c, pct);

        // Set the alpha
        c.a = U8(alpha);

        // Render the display
        ObjectGroundSprite(obj, c, tex);
      }
    }


    //
    // Get an identified mesh
    //
    MeshRoot * GetMesh(U32 id)
    {
      return (meshes.Find(id));
    }


    //
    // Render a mesh with a color
    //
    void Mesh(U32 id, const Matrix &m, const Color &c)
    {
      if (MeshRoot *mesh = meshes.Find(id))
      {
        mesh->RenderLightSun(const_cast<Matrix &>(m), c);
      }
    }


    //
    // Render a mesh without a color
    //
    void Mesh(U32 id, const Matrix &m)
    {
      if (MeshRoot *mesh = meshes.Find(id))
      {
        mesh->RenderLightSun(const_cast<Matrix &>(m));
      }
    }


    //
    // MarkPosition
    //
    // Mark a terrain position with an arrow
    //
    void MarkPosition(F32 x, F32 z, Color color)
    {
      if (WorldCtrl::MetreOnMap(x, z))
      {
        // Calculate the x, y, z position of the centre of the supplied cell
        Matrix  m;
        m.ClearData();

        m.posit.x = x;
        m.posit.z = z;
        m.posit.y = TerrainData::FindFloorWithWater(x, z, &m.up);
        m.SetFromUp(m.up);

        MarkPosition(m, color);
      }
    }


    //
    // Mark a world position with an arrow
    //
    void MarkPosition(const Vector &pos, Color color)
    {
      Matrix m;
      m.ClearData();
      m.posit = pos;

      MarkPosition(m, color);
    }


    //
    // Mark a world position using the given matrix
    //
    void MarkPosition(const Matrix &matrix, Color color)
    {
      Common::Display::Mesh(0x92098DF3, matrix, color); // "TerrainMarker"    
    }


    //
    // MarkCell
    //
    // Mark a cell with an arrow
    //
    void MarkCell(U32 cx, U32 cz, Color color)
    {
      MarkPosition(
        WorldCtrl::MetreMapXMin() + (((F32) cx) + 0.5f) * WorldCtrl::CellSize(),
        WorldCtrl::MetreMapZMin() + (((F32) cz) + 0.5f) * WorldCtrl::CellSize(),
        color);
    }


    //
    // MarkAreaCell
    //
    // Marks cells on the perimeter of an area
    //
    void MarkAreaCell(const Area<U32> &area, Color *colors)
    {
      Area<F32> f
      (
        WorldCtrl::CellToMetresX(area.p0.x),
        WorldCtrl::CellToMetresZ(area.p0.z),
        WorldCtrl::CellToMetresX(area.p1.x),
        WorldCtrl::CellToMetresZ(area.p1.z)
      );

      MarkAreaMetre(f, colors);
    }


    //
    // MarkAreaMetre
    //
    // Marks cells on the perimeter of an area
    //
    void MarkAreaMetre(const Area<F32> &area, Color *colors)
    {
      const U32 MAXPOINTS = 8;

      Color corner = colors ? colors[0] : Color(U32(0), U32(255), U32(255));
      Color centre = colors ? colors[1] : Color(U32(255), U32(255), U32(0));
      Color line   = colors ? colors[2] : Color(U32(0), U32(127), U32(127));

      // Get the width and height of the region
      F32 width = area.Width();
      F32 height = area.Height();

      // Work out how may points will be used vertically and horizontally
      U32 horizPoints = (U32) (width / 5.0f);
      U32 vertPoints = (U32) (height / 5.0f);

      if (horizPoints && vertPoints)
      {
        horizPoints = Min<U32>(horizPoints, MAXPOINTS);
        vertPoints = Min<U32>(vertPoints, MAXPOINTS);

        // Work out the point separation
        F32 horizSep = width / (F32) horizPoints;
        F32 vertSep = height / (F32) vertPoints;

        // Draw Lines
        F32 x = area.p0.x;
        while (--horizPoints)
        {
          x += horizSep;
          MarkPosition(x, area.p0.y, line);
          MarkPosition(x, area.p1.y, line);
        }

        F32 y = area.p0.y;
        while (--vertPoints)
        {
          y += vertSep;
          MarkPosition(area.p0.x, y, line);
          MarkPosition(area.p1.x, y, line);
        }
      }

      // Draw Corners
      MarkPosition(area.p0.x, area.p0.y, corner);
      MarkPosition(area.p1.x, area.p0.y, corner);
      MarkPosition(area.p0.x, area.p1.y, corner);
      MarkPosition(area.p1.x, area.p1.y, corner);

      // Draw Centre
      MarkPosition(area.p0.x + area.Width() * 0.5F, area.p0.z + area.Height() * 0.5F, centre);
    }


    //
    // MarkCircleMetre
    //
    // Marks cells on the perimeter of a circle
    //
    void MarkCircleMetre(const Point<F32> &point, F32 radius, const Color &color)
    {
#if 1
      if (!Terrain::MeterOnMap( point.x, point.y))
      {
        return;
      }
      U32 maxPoints = U32(radius * 0.7f) + 1;

      Vector * points;
      U32 heapSize = Vid::Heap::ReqVector( &points, maxPoints);

      F32 baseAngle = F32(fmod(F32(Clock::Time::Ms()) * 0.0001f, PI2));

      const F32 dh = 2;
      F32 height = TerrainData::FindFloorWithWater( point.x, point.y) + dh;

      Vector * p, * pe = points + maxPoints - 1;
      F32 count = 0;
      for (p = points; p < pe; p++, count++)
      {
        F32 angle = count * PI2 / F32(maxPoints) - PI + baseAngle;

        p->x = point.x + F32(cos(angle)) * radius;
        p->z = point.y + F32(sin(angle)) * radius;

        if (Terrain::MeterOnMap( p->x, p->z))
        {
          p->y = Terrain::FindFloorWithWater( p->x, p->z) + dh;
        }
        else
        {
          p->y = height;
        }
      }

      Bitmap * texture = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, "bolt.tga");

      // complete the circle
      *p = *points;

      Vector camPos;
      Vid::TransformFromWorld( camPos, Vector(point.x, height, point.y));

      Vid::RenderBeam( TRUE, 
        points, maxPoints, 1, camPos.z, 
        texture, color, RS_BLEND_ADD, Vid::sortEFFECT0,
        0, .02f, FALSE);

      Vid::Heap::Restore( heapSize);
#else
      U32 points;
      U32 maxPoints = U32(radius * 0.3f);

      F32 baseAngle = F32(fmod(F32(Clock::Time::Ms()) * 0.0001f, PI2));

      for (points = 0; points < maxPoints; points++)
      {
        F32 angle = F32(points) * PI2 / F32(maxPoints) - PI + baseAngle;

        F32 x = point.x + F32(cos(angle)) * radius;
        F32 y = point.y + F32(sin(angle)) * radius;

        MarkPosition(x, y, color);
      }
#endif
    }


    //
    // MarkGrain
    //
    // Draw a rectangle around a grain
    //
    void MarkGrain(S32 x, S32 z, Color c, F32 height)
    {
      static const F32 FATNESS = 0.4F;

      F32 mx = x * 4.0F;
      F32 mz = z * 4.0F;
      Vector v[4];

      v[0].x = mx + 0.25F;
      v[0].z = mz + 0.25F;
      v[1].x = mx + 3.75F;
      v[1].z = mz + 0.25F;
      v[2].x = mx + 0.25F;
      v[2].z = mz + 3.75F;
      v[3].x = mx + 3.75F;
      v[3].z = mz + 3.75F;

      for (U32 i = 0; i < 4; i++)
      {
        WorldCtrl::ClampMetreMap(v[i].x, v[i].z);
        v[i].y = TerrainData::FindFloor(v[i].x, v[i].z) + height;
      }

      Render::FatLine(v[0], v[1], c, FATNESS);
      Render::FatLine(v[0], v[2], c, FATNESS);
      Render::FatLine(v[3], v[1], c, FATNESS);
      Render::FatLine(v[3], v[2], c, FATNESS);
    }


    //
    // Add a marker
    //
    void AddMarker(S32 x, S32 z, F32 value)
    {
      // Build marker key
      U32 key = (x << 16) | z & 0xFFFF;

      // Check to see if there's an existing marker
      Marker *marker = markers.Find(key);

      if (marker)
      {
        // If it does exist update the value
        marker->value = value;
      }
      else
      {
        // Add a new marker
        markers.Add(key, new Marker(value));
      }
    }


    //
    // Remove a marker
    //
    void RemoveMarker(S32 x, S32 z)
    {
      // Build marker key
      U32 key = (x << 16) | z & 0xFFFF;

      // Check to see if there's an existing marker
      Marker *marker = markers.Find(key);

      if (marker)
      {
        // Dispose of the marker
        markers.Dispose(marker);
      }
    }


    //
    // Clear markers
    //
    void ClearMarkers()
    {
      markers.DisposeAll();
    }


    //
    // Render markers
    //
    void RenderMarkers()
    {
      for (NBinTree<Marker>::Iterator m(&markers); *m; m++)
      {
        U32 key = m.GetKey();

        S32 x = (key >> 16) & 0xFFFF;
        S32 z = key & 0xFFFF;

        Vector v;
        v.x = WorldCtrl::MetreMapXMin() + (((F32) x) + 0.5f) * WorldCtrl::CellSize();
        v.z = WorldCtrl::MetreMapZMin() + (((F32) z) + 0.5f) * WorldCtrl::CellSize();
        v.y = TerrainData::FindFloorWithWater(v.x, v.z);

        F32 value = (*m)->value;

        Render::FatLine(v, v + Vector(0.0f, 10.0f * value, 0.0f), Color(1.0f - value, value, 0.0f), 0.5f);
      }
    }


    //
    // FootPrintPlacement
    //
    // Display a footprint placement grid
    //
    void FootPrintPlacement(FootPrint::Placement &placement, Bool mesh, Matrix *matrix, F32 alpha)
    {
      Bool allok = TRUE;

      // Scale the alpha down
      alpha = alpha * 0.5f + 0.25f;

      // Display each cell
      for (S32 z = 0; z < placement.Size().z; z++)
      {
        for (S32 x = 0; x < placement.Size().x; x++)
        {
          // Get the cell at this position
          FootPrint::Placement::Cell &cell = placement.GetCell(x, z);

          // Is this cell acceptable
          Bool ok = (cell.result == FootPrint::Placement::PR_OK);

          if (!ok)
          {
            allok = FALSE;
          }

          // Only display fringe cells if they have a negative result
          if ((cell.onFoot & !mesh) || !ok)
          {
            // Get the actual terrain position
            if (WorldCtrl::CellOnMap(cell.map.x, cell.map.z))
            {
              Area<S32> rect;
              rect.SetSize(cell.map.x, cell.map.z, 1, 1);

              // Set the intensity modifier
              F32 i = (!mesh && z == placement.Size().z - 2) ? 0.7f : 0.0f;

              // Setup the color based on the result
              F32 r = ok ? 0.0f : 0.3f;
              F32 g = 0.0f;
              F32 b = ok ? 0.3f : 0.0f;

              // Add in intensity
              if (r) { r += i; }
              if (g) { g += i; }
              if (b) { b += i; }

              // Set the alpha value
              F32 a = alpha * ((z == placement.Size().z - 2) ? 0.9f : 0.6f);
     
              // Render the cell
              Terrain::RenderCellRect(rect, Color(r, g, b, 1.0f - a), TRUE);
            }
          }
        }
      }

      // Render the mesh
      if (mesh)
      {
        FootPrintPlacementMesh
        (
          placement, 
          matrix, 
          allok ? 
            Color(0.5f, 0.5f, 0.5f, alpha) :
            Color(1.0f, 0.0f, 0.0f, alpha)
        );
      }
    }


    //
    // FootPrintPlacementDeform
    //
    // Display a footprint placement grid with terrain deformation
    //
    void FootPrintPlacementDeform(FootPrint::Placement &placement, PlacementDeform &deform, Bool mesh, Matrix *matrix, F32 alpha)
    {
      ASSERT(!deform.count)

      // Scale the alpha down
      alpha = alpha * 0.5f + 0.25f;

      const FootPrint::Placement::Thumped &thump = placement.GetThumped();
      Bool allok = TRUE;

      // Display each cell
      for (S32 z = 0; z < placement.Size().z; z++)
      {
        for (S32 x = 0; x < placement.Size().x; x++)
        {
          // Get the cell at this position
          FootPrint::Placement::Cell &cell = placement.GetCell(x, z);

          // Is this cell acceptable
          Bool ok = (cell.result == FootPrint::Placement::PR_OK);

          if (!ok)
          {
            allok = FALSE;
          }

          // Convert this location to the footprint location
          Point<S32> thumpPos = cell.map - placement.GetMin() + Point<S32>(1, 1);

          if (thump.IsValid())
          {
            Vector v[4];

            // Hide the terrain cell?
            ::Cell *terrainCell = Terrain::GetCell(cell.map.x, cell.map.z);

            // Toggle cell display
            if (deform.count < PlacementDeform::MAX_PTS)
            {
              if (terrainCell->GetVisible())
              {
                terrainCell->SetVisible(FALSE);
                deform.points[deform.count++] = cell.map;
              }
            }

            F32 cx0 = F32(cell.map.x) * WC_CELLSIZEF32;
            F32 cz0 = F32(cell.map.z) * WC_CELLSIZEF32;
            F32 cx1 = cx0 + WC_CELLSIZEF32;
            F32 cz1 = cz0 + WC_CELLSIZEF32;

            v[0].Set(cx0 , thump.GetHeight(thumpPos.x, thumpPos.z), cz0);
            v[1].Set(cx0 , thump.GetHeight(thumpPos.x, thumpPos.z+1), cz1);
            v[2].Set(cx1 , thump.GetHeight(thumpPos.x+1, thumpPos.z+1), cz1);
            v[3].Set(cx1 , thump.GetHeight(thumpPos.x+1, thumpPos.z), cz0);

            Vid::SetWorldTransform(Matrix::I);
            Vid::SetBucketPrimitiveDesc
            (
              PT_TRIANGLELIST, 
              FVF_TLVERTEX, DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | RS_BLEND_DEF 
            );
            Vid::SetBucketMaterial(Vid::defMaterial);
            Vid::SetBucketTexture( NULL, TRUE);

            Color clrOk = Color(128L, 128L, 255L);
            Color clrBad = Color(128L, 0L, 64L);

            Vid::ProjectClip( v, NULL, ok ? clrOk : clrBad, RGBA_MAKE(0x7F, 0x7F, 0x7F, 0xFF), 4, Vid::rectIndices, 6, FALSE);
          }
        }
      }

      // Render the mesh
      if (mesh)
      {
        FootPrintPlacementMesh
        (
          placement, 
          matrix, 
          allok ? 
            Color(0.5f, 0.5f, 0.5f, alpha) :
            Color(1.0f, 0.0f, 0.0f, alpha)
        );
      }
    }


    //
    // PostFootPrintPlacementDeform
    //
    // Clean up aftera FootPrintPlacementDeform
    //
    void PostFootPrintPlacementDeform(PlacementDeform &deform)
    {
      ASSERT(deform.count < PlacementDeform::MAX_PTS)

      for (U32 i = 0; i < deform.count; i++)
      {
        ::Cell *terrainCell = Terrain::GetCell(deform.points[i].x, deform.points[i].z);
        terrainCell->SetVisible(TRUE);
      }

      deform.count = 0;
    }


    //
    // FootPrintPlacementMesh
    //
    // Display the mesh of a footprint placement grid
    //
    void FootPrintPlacementMesh(FootPrint::Placement &placement, Matrix *matrix, const Color &color)
    {
      ASSERT(matrix)

      MeshRoot *root = placement.GetType().GetMapType().GetMeshRoot();

      if (root)
      {
        root->RenderColor(*matrix, color);
      }
    }


    //
    // GetTrailPoint
    //
    // Returns the world position for the given cell point
    //
    static const Vector & GetTrailPoint(const TrailObj::WayPoint &p, F32 height)
    {
      static Vector v;

      v.x = WorldCtrl::CellToMetresX(p.x);
      v.z = WorldCtrl::CellToMetresZ(p.z);
      v.y = TerrainData::FindFloorWithWater(v.x, v.z) + height;

      return (v);
    }

    
    // 
    // RenderTrailSegment
    //
    // Render a single trail segment
    //
    static void RenderTrailSegment(const Vector &p0, const Vector &p1)
    {
      Matrix src;
      src.ClearData();
      src.posit = p0;

      // Do we have the particle
      if (trailParticle.Alive())
      {
        trailParticle->Update(src, p1 - p0);
      }
      else
      {
        // Find the trail particle class
        if (ParticleClass *p = ParticleSystem::FindType(0xBD849BAC)) // "Common::Trail"
        {
          Vector v(0.0F, 0.0F, 0.0F);

          // Create a particle with default data
          trailParticle = ParticleSystem::New(p, src, v, v, p1 - p0, 0.0F);
        }
      }

      // Do we have a trail particle
      if (trailParticle.Alive())
      {
        trailParticle->SetupRenderers();
        trailParticle->Render();
      }   
    }


    //
    // WayPoints
    //
    // Display a list of waypoints
    //
    void WayPoints(const TrailObj::WayPointList &list, const Color &c, TrailObj::Mode mode)
    {
      Bool previous = FALSE;
      Vector pt0, pt1, first;
      pt1.ClearData();

      for (TrailObj::WayPointList::Iterator i(&list); *i; i++)
      {
        // Get the point
        const TrailObj::WayPoint &w = **i;

        // Render the mesh
        Matrix m;
        m.ClearData();
        m.posit = GetTrailPoint(w, 0.0F);
        Mesh(0x693D5359, m, c); // "Location"

        // Shuffle point down
        pt0 = pt1;
        pt1 = m.posit;

        // Lift the beams
        pt1.y += 3.0F;

        if (previous)
        {
          RenderTrailSegment(pt0, pt1);
        }
        else
        {
          // Save first point
          first = pt1;
        }

        previous = TRUE;
      }

      if (mode == TrailObj::MODE_LOOPIN && previous && list.GetCount() > 2 && first != pt1)
      {
        RenderTrailSegment(pt1, first);
      }
    }


    //
    // RenderDebug
    //
    // Render debugging info
    //
    void RenderDebug()
    {
#ifdef DEVELOPMENT
      if (Common::Debug::data.claiming)
      {
        Claim::RenderDebug();
      }

      if (Common::Debug::data.thumpMutex)
      {
        BitArray2d *p = FootPrint::GetThumpMutex();
        Color clr(255L, 128L, 128L);

        for (U32 z = 0; z < WorldCtrl::CellMapZ(); z++)
        {
          for (U32 x = 0; x < WorldCtrl::CellMapX(); x++)
          {
            if (p->Get2(x, z))
            {
              MarkCell(x, z, clr);
            }
          }
        }
      }

      if (Vid::Var::mirrorDebug)
      {
        Terrain::WaterRegion * wr, * we = Terrain::waterList.data + Terrain::waterCount; 
        for (wr = Terrain::waterList.data; wr < we; wr++)
        {
          Vid::RenderRectangle( wr->mRect, 0x55ff0000);
        }
      }
#endif

      RenderMarkers();

      // Eeeek ... butt ugly ... but there's no other way
      /*
      if (Common::Debug::data.explosion)
      {
        for (NList<MapObj>::Iterator mi(&MapObjCtrl::GetOnMapList()); *mi; mi++)
        {
          // If its an explosion then do an explosion render debug
          ExplosionObj *explosion = Promote::Object<ExplosionObjType, ExplosionObj>(*mi);

          if (explosion)
          {
            explosion->RenderDebug();
          }
        }
      }
      */
    }


    //
    // CmdHandler
    //
    // Handles var system events
    //
    static void CmdHandler(U32)
    {
      ASSERT(initialized);
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      ASSERT(!initialized);
      ASSERT(!initializedSim);

      VarSys::RegisterHandler("common.display", CmdHandler);

      // Display vars
      VarSys::CreateInteger("common.display.clear", 0, VarSys::DEFAULT, &Display::clear);
      VarSys::CreateInteger("common.display.bright", 0, VarSys::DEFAULT, &Display::bright);

      // Clear data
      trailParticle = NULL;

      initializedSim = FALSE;
      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      if (initialized)
      {
        ASSERT(!initializedSim);

        VarSys::DeleteItem("common.display");

        // Clear any remaining marks
        ClearMarkers();

        initialized = FALSE;
      }
    }    


    //
    // InitSim
    //
    // Initialize this sub-system
    //
    void InitSim()
    {
      ASSERT(initialized);
      ASSERT(!initializedSim);

      // Load bitmap resources
      textures.Add(0x1483BCF7, Bitmap::Manager::FindCreate( Bitmap::reduceNONE, "engine_healthbar.tga")); // "HealthBar"

      // Load mesh resources
      MeshRoot *root;

      root = Mesh::Manager::FindRead("engine_arrow.god", 1.0f, FALSE);
      meshes.Add(0x92098DF3, root); // "TerrainMarker"

      root = Mesh::Manager::FindRead("engine_formation.god", 1.0f, FALSE);
      meshes.Add(0x7223612A, root); // "Formation"

      root = Mesh::Manager::FindRead("engine_location.god", 1.0f, FALSE);
      meshes.Add(0x693D5359, root); // "Location"

      initializedSim = TRUE;
    }


    //
    // DoneSim
    //
    // Shutdown this sub-system
    //
    void DoneSim()
    {
      ASSERT(initialized);
      ASSERT(initializedSim);

      // Free all resources
      meshes.UnlinkAll();
      textures.UnlinkAll();

      initializedSim = FALSE;
    }    
  }
}
