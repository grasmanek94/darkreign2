///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


#ifndef __COMMON_GAMEWINDOW_H
#define __COMMON_GAMEWINDOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "unitobjdec.h"
#include "team.h"


//
// Forward declarations
//
struct Cell;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace GameWindowMsg
  //
  namespace GameWindowMsg
  {
    const U32 Refresh     = 0xFB252575; // "GameWindow::Message::Refresh"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class GameWindow
  //
  class GameWindow : public IControl
  {
    PROMOTE_LINK(GameWindow, IControl, 0xA78DC8BF); // "GameWindow"

  public:

    // Handler for events
    typedef Bool (EventCallBack)(Event &e, GameWindow *gameWindow);

    // Handler for object selection
    typedef MapObj * (SelectFilter)(MapObj *obj);

  private:

    // Registered event handler for ALL game windows, or NULL if none
    static EventCallBack *handler;

    // Returns TRUE if the given position is within the given rect
    Bool RectTest(const Vector &position, const Area<S32> &rect, Vector &scr);

  public:

    // Register the game window event handler
    static EventCallBack *RegisterHandler(EventCallBack *function);

  public:

    // Constructor
    GameWindow(IControl *parent);
 
    // Event handling
    U32 HandleEvent(Event &e);

    // Add a unit object to the given list (TRUE if added)
    static Bool AddToSelected
    (
      UnitObj *obj, UnitObjList &list, Team *team = NULL, 
      Bool allowFoot = TRUE, Bool selectParent = TRUE
    );

    // Find an object at the screen pixel position x,y
    virtual MapObj * PickObject(S32 x, S32 y, SelectFilter *filter = NULL);

    // Do a group object selection using the given screen rectangle
    virtual Bool SelectGroup(const Area<S32> &rect, MapObjList &list, SelectFilter *filter = NULL);

    // Do a group object selection using the given screen rectangle
    virtual Bool SelectGroup(const Area<S32> &rect, UnitObjList &list, Team *team = NULL);

    // Returns the closest object to the centre of, and within, 'rect'
    virtual MapObj * FindClosest(const Area<S32> &rect);

    // Returns the terrain cell and metre position at screen x,y (or NULL)
    virtual Bool TerrainPosition(S32 x, S32 y, Vector &pos);

    // Returns TRUE if this game window has the specified property
    virtual Bool HasProperty(U32 propertyCrc);

    // String access for above method
    Bool HasProperty(const char *propertyName)
    {
      return (HasProperty(Crc::CalcStr(propertyName)));
    }
  };
}

#endif
