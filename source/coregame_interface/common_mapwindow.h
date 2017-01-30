///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


#ifndef __COMMON_MAPWINDOW_H
#define __COMMON_MAPWINDOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_gamewindow.h"
#include "mapobjdec.h"
#include "terrain.h"
#include "iface_util.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MapWindow
  //
  class MapWindow : public GameWindow
  {
    PROMOTE_LINK(MapWindow, GameWindow, 0xB57D2BB8); // "MapWindow"

  public:

    // The rotation of the map
    F32 rotation;

    // Viewing distance of minimap
    F32 viewDist;

    // Texture scale
    VarFloat scaleFactor;

    // Transformation matrix for world->normalised client coords
    F32 m[6];

    // Rotating minimap is centred at the current camera pos
    U32 rotating    : 1,  
        fixed       : 1,
        angleHack   : 1,
                      
    // Scale minimap using view dist
        scaleToDist : 1;  

    // FOV color
    Color fovColor0;
    Color fovColor1;

    // Custom map orientation
    Vector mapFront;
    Vector mapRight;

    // FOV texture
    TextureInfo fovTexture;

    // FOV size
    F32 fovSize;

    // Blip texture
    TextureInfo blipTexture;

    // Blip size
    F32 blipSize;

  protected:

    // Draw self
    void DrawSelf(PaintInfo &pi);

    // Fill unit array for the given team (TRUE if array is full)
    static Bool ProcessUnitsOnTeam(Team *team);

    // Draw units onto terrain texture
    static void UpdateTexture();

    // Default lifetime of a blip
    static F32 blipTime;
    static F32 blipTimeInv;

    // Map availability at last draw
    static S32 wasAvailable;

  public:

    // Initialise/shutdown map window
    static void Init(IFace::PulsingValue *alpha = NULL);
    static void Done();

    // Render the terrain into the offscreen buffer
    static void RenderTerrain();

    // Trigger a blip on the map
    static void Blip(const Vector &location, Color color, F32 persistTime);

    // Line of sight display mode has changed
    static void LOSDisplayChanged();

    // Is radar available
    static Bool Available();

  public:

    // Constructor and destructor
    MapWindow(IControl *parent);
    ~MapWindow();

    // Control activation
    Bool Activate();
    Bool Deactivate();

    // Configure this control with an FScope
    void Setup(FScope *fScope);

    // Event handler
    U32 HandleEvent(Event &e);

    // Find an object at the screen pixel position x,y
    MapObj * PickObject(S32 x, S32 y, SelectFilter *filter = NULL);

    // Do a group object selection using the given screen rectangle
    Bool SelectGroup(const Area<S32> &rect, MapObjList &list, SelectFilter *filter = NULL);

    // Do a group object selection using the given screen rectangle
    Bool SelectGroup(const Area<S32> &rect, UnitObjList &list, Team *team = NULL);

    // Returns the closest object to the centre of, and within, 'rect'
    MapObj * FindClosest(const Area<S32> &rect);

    // Returns the terrain cell and metre position at screen x,y (or NULL)
    Bool TerrainPosition(S32 x, S32 y, Vector &pos);

    // Returns TRUE if this game window has the specified property
    Bool HasProperty(U32 propertyCrc);

  public:  

    // Save the current map to the mission folder
    static void Save(const char *path);

  };
}

#endif
