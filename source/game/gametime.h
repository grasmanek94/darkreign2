///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Time System
// 15-JUL-1998
//


#ifndef __GAMETIME_H
#define __GAMETIME_H


///////////////////////////////////////////////////////////////////////////////
//
// Inclues
//
#include "varsys.h"
#include "mono.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace GameTime
//
namespace GameTime
{
  const U32 CYCLESPERSECOND = 10;
  const U32 INTERVALUS = (1000000 / CYCLESPERSECOND);
  const F32 INTERVAL = (1.0f / F32(CYCLESPERSECOND));
  const F32 INTERVALINV = (F32(CYCLESPERSECOND));

  extern U32 gameCycle;
  extern F32 simTotalTime;
  extern U32 simCycle;
  extern S32 displayCount;
  extern S32 displayTotalCount;
  extern F32 interpolationTime;
  extern Bool aborted;


  // Init: Initialized game time system
  void Init();

  // Done: Shutdown game time system
  void Done();

  // Reset: Resets the game time system
  void Reset();

  // Save and load system data
  void Save(FScope *scope);
  void Load(FScope *scope);

  // GameCycle: Returns the current game cycle
  inline U32 GameCycle()
  {
    return (gameCycle);
  }

  // SimTime: Returns the simulation time for the current game cycle
  inline F32 SimTime() 
  { 
    return (INTERVAL); 
  }

  // SimTimInv: Returns the inverse of the simulation time for the current game cycle
  inline F32 SimTimeInv()
  {
    return (INTERVALINV);
  }

  // SimTotalTime: Returns the total amount of simulated time so far
  inline F32 SimTotalTime() 
  { 
    return (simTotalTime); 
  }

  // SimCycle: Returns the current simulation cycle
  inline U32 SimCycle()
  {
    return (simCycle);
  }

  // DisplaySample: A sample of how long it takes to do a display cycle has been taken
  void DisplaySample(U32 sample);

  // GameCycleOffset: Returns "interpolated" offset from last game cycle (in seconds)
  F32 GameCycleOffset();

  // DisplayCount: Returns the number of display cycles remaining in this game cycle
  inline S32 DisplayCount() 
  { 
    return (displayCount); 
  }

  // DisplayTotalCount: Returns the total number of display cycles to di in thie game cycle
  inline S32 DisplayTotalCount() 
  { 
    return (displayTotalCount); 
  }

  // Aborted: Returns true if this game cycle occured by aborting some of the display cycles
  inline U32 Aborted() 
  { 
    return (aborted); 
  }

  // InterpolationTime: Returns the interpolation step
  inline F32 InterpolationTime()
  {
    return (interpolationTime);
  }

  // TRUE if simulation is taking too long and displays are being forced
  Bool IsFallingBehind();

  // TRUE if too many display frames are being dropped
  Bool IsDroppingFrames();

  // TRUE if stalled
  Bool IsStalled();

  // Test: Test to see if we should proceed to the next game cycle
  Bool Test();

  // Compute: Calculate the number of display cycles we can perform in a game cycle
  void Compute();
  
  // Cycle: Increment to the next game cycle
  Bool Cycle(Bool force = FALSE);

  // Pause: Pause/Unpause the game
  void Pause(Bool triggerMessage = TRUE);

  // Paused: Return the current paused state
  Bool Paused();

  // StepOnce: Single Step the game (effectively, pause next cycle)
  void StepOnce();

  // Readjust: Adjust gametime back to real time, after unpausing a single player game
  void Readjust();

  // Sync: Synchronize network cycle with game cycle - used for multiplayer joins
  void Synchronize();

  // SetFastMode: Enable/disable fast simulation
  void SetFastMode(Bool flag);

  // GetFastMode: Return current fast mode flag (cineractive is skipping forward)
  Bool GetFastMode();

  // GetFastForwardMode: Return fast forward flag (user is holding down fast forward)
  Bool GetFastForwardMode();  

  // SetDisplayMode: Enable/disable display
  void SetDisplayMode(Bool flag);

  // GetDisplayMode: Return current display mode flag
  Bool GetDisplayMode();

  // Returns the number of seconds since the given sim cycle
  F32 TimeSinceCycle(U32 cycle);


  /////////////////////////////////////////////////////////////////////////////
  // 
  // Class Timer 
  //
  class Timer
  {
  private:

    // Time that the timer should go off
    U32 triggerTime;

  public:

    Timer() : triggerTime(0) {}

    // Set the timer to go off in "time" seconds
    void Start(F32 time);

    // Test if the timer has gone off
    Bool Test();

    // Time remaining
    F32 GetRemaining() const;

    // Reset the timer
    void Reset();

    // Save and load
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);
  };

};

#endif