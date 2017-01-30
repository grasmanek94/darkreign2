///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_brush_water.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Water - Used to edit water heights
    //

    //
    // Constructor
    //
    Water::Water(const char *name) : AreaBase(name)
    {
      // Create interface vars
      varHeight = CreateFloat("Height", 0.0F, F32_MIN, F32_MAX);
    }


    //
    // Destructor
    //
    Water::~Water()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Water::HasProperty(U32 property)
    {
      return (AreaBase::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Water::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      AreaBase::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Water::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x97D6B2EC: // "Brush::Water::Message::Apply"
        case 0x5AC93AF7: // "Brush::Water::Message::Clear"
        {
          if (valid)
          {
            // Start a modification session
            TerrainData::SessionStart();

            // Ensure sorted points
            if (metreArea.p0.x > metreArea.p1.x) { Swap(metreArea.p0.x, metreArea.p1.x); }
            if (metreArea.p0.z > metreArea.p1.z) { Swap(metreArea.p0.z, metreArea.p1.z); }

            // Get cluster positions
            Point<U32> c0, c1;
            Terrain::GetClusterOffsets(metreArea.p0.x, metreArea.p0.z, c0.x, c0.z);
            Terrain::GetClusterOffsets(metreArea.p1.x, metreArea.p1.z, c1.x, c1.z);

            // Iterate over each cluster
            for (U32 z = c0.z; z <= c1.z; z++)
            {
              for (U32 x = c0.x; x <= c1.x; x++)
              {
                // Get this cluster
                Cluster &clus = Terrain::GetCluster(x, z);

                if (crc == 0x97D6B2EC) // "Brush::Water::Message::Apply"
                {
                  // Ensure water is on
                  clus.status.water = TRUE;

                  // Set new height
                  clus.waterHeight = varHeight->GetFloatValue();
                }
                else
                {
                  clus.status.water = FALSE;
                }
              }
            }

            // Notify that area has changed
            Point<S32> cell0, cell1;

            cell0.x = WorldCtrl::ClusterToLeftCell(c0.x);
            cell0.z = WorldCtrl::ClusterToTopCell(c0.z);
            cell1.x = WorldCtrl::ClusterToRightCell(c1.x);
            cell1.z = WorldCtrl::ClusterToBottomCell(c1.z);

            TerrainData::SessionAddArea(cell0, cell1);
            TerrainData::SessionEnd();
          }

          // Block this event
          return;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Is mouse over the terrain
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            F32 height = TerrainData::FindFloor
            (
              data.cInfo.mTerrain.pos.x, data.cInfo.mTerrain.pos.z
            );

            if (Common::Input::GetModifierKey(3))
            {
              TerrainData::FindWater
              (
                data.cInfo.mTerrain.pos.x, data.cInfo.mTerrain.pos.z, &height
              );
            }

            varHeight->SetFloatValue(height);
          }

          // Block this event
          return;
        }
      }

      // Not blocked at this level
      AreaBase::Notification(crc, e);
    }
  }
}
