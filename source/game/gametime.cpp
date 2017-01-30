///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Time System
// 15-JUL-1998
//

//
// The game time system is responsible for controlling the speed of the game
// and maintaining constant game speed ... it is also responsible for inserting
// networked vars into the game in a synchronous fashion
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gametime.h"
#include "multiplayer_data.h"
#include "console.h"
#include "orders.h"
#include "sync.h"
#include "message.h"
#include "main.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace GameTime
//
namespace GameTime
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Current game cycle
  U32 gameCycle;

  U32 lastTime;

  // Simulation time since the game started
  F32 simTotalTime;

  // Simulation cycle
  U32 simCycle;

  // Number of display cycles remainin in this game cycle
  S32 displayCount;

  // Total number of display cycles to do in this game cycle
  S32 displayTotalCount;

  // Did we not do all of the display cycles in the last game cycle ?
  Bool aborted;

  // Time step for next frame of interpolation
  F32 interpolationTime;

  // Has the system been initialized
  static Bool initialized = FALSE;

  // Time when the game was reset (us)
  static U32 timeFirstGameCycle;

  // Time when next game cycle should occur (us)
  static U32 timeNextGameCycle;

  // Time it takes to perform a display cycle (us)
  static U32 displayTime;

  // Deviation in display time (us)
  static U32 displayTimeDev;

  // Current network cycle
  static U32 nextNetworkGameCycle;

  // Current network sequence number
  static U32 networkSequence;

  // Are we waiting for network orders ?
  static Bool stalled;

  // Lag indicator turns on after this amount of continuous stalls
  static const U32 STALL_THRESHOLD = 750000;

  // TRUE if stalls are persistent
  static Bool persistentStalls;

  // What time did we start waiting for network orders ?
  static Bool networkStallTime;

  // Are we paused ?
  static Bool paused;

  // Are we single stepping ?
  static Bool stepOnce;

  // Is Fast mode enabled?
  static Bool fastMode;
  static Bool fastForwardMode;

  // Is display enabled?
  static Bool displayMode;

  // Fast forward mode
  static VarInteger fastForward;

  // Fast display mode
  static VarInteger fastDisplay;

  // Amount to bump when we detect we're out
  static const U32 BUMPTIMEPOSUS = 5000;
  static const U32 BUMPTIMENEGUS = 5000;

  // Minimum amount of time between bumps in different directions(10 sec)
  static const U32 BUMPINTERVALUS = 10000; 

  // Last time a positive bump occured  
  static U32 lastPosBumpTime;           

  // Last time a negative bump occured
  static U32 lastNegBumpTime;

  // Number of positive bumps which have occured
  static U32 numPosBumps;            

  // Number of negative bumps which have occured
  static U32 numNegBumps;            

  // Display is dropping too many frames when this percentage of frames are skipped
  static const F32 DISPLAY_SKIP_THRESHOLD_HI = 0.25F;
  static const F32 DISPLAY_SKIP_THRESHOLD_LO = 0.15F;

  // Consecutive cycles that display has been aborted
  static U32 consecutiveAborts;

  // TRUE if simulation is falling further behind real time
  static Bool fallingBehind;

  // Percentage of sim cycles that don't have a corresponding display
  static F32 displaySkipRate;

  // TRUE when displaySkipRate >= DISPLAY_SKIP_THRESHOLD
  static Bool droppingFrames;

  // Low pass filter rates
  static const F32 Filter1 = 0.90F;
  static const F32 Filter2 = 1.0F - Filter1;
  

  // Mono buffer
  MonoBufDef(monoBuffer);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //

  // IncGameCycle: Increment the game cycle
  void IncGameCycle();

  // CmdHandler: Command handler for the game timing system
  void CmdHandler(U32 pathCrc);


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized);

    // Creating Timing Scope
    VarSys::RegisterHandler("timing", CmdHandler);//, VarSys::NETWORK | VarSys::QUEUE);

    // Reset
    VarSys::CreateCmd("timing.reset");

    // Report
    VarSys::CreateCmd("timing.report");

    // Pause
    VarSys::CreateCmd("timing.pause");

    // StepOnce
    VarSys::CreateCmd("timing.steponce");

    // Readjust
    VarSys::CreateCmd("timing.readjust");

    // Fast forward setting
    VarSys::CreateInteger("timing.fastforward", FALSE, VarSys::NOTIFY, &fastForward)->SetIntegerRange(0, 1);
    VarSys::CreateInteger("timing.fastdisplay", FALSE, VarSys::NOTIFY, &fastDisplay)->SetIntegerRange(0, 1);

    // Create Mono Buffer
    MonoBufCreate("GameTime Panel", &monoBuffer);

    // Now initialized
    initialized = TRUE;

    // Reset system data
    Reset();
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized);

    // Destroy game time scope
    VarSys::DeleteItem("timing");

    // Destroy Mono Buffer
    MonoBufDestroy(&monoBuffer);

    // Now shutdown
    initialized = FALSE;
  }


  //
  // Reset 
  //
  void Reset()
  {
    ASSERT(initialized)

    lastTime = 0;

    // Game cycle

    // The game cycle is reset to zero
    gameCycle = 0;

    // The next network cycle will occur in GAMETIME_INTERVAL time
    timeFirstGameCycle = Clock::Time::UsLwr();
    timeNextGameCycle = timeFirstGameCycle + INTERVALUS;

    // Simulation time

    // Total simiulated time so far
    simTotalTime = 0;

    // Simulation cycle
    simCycle = 0;

    // Display cycle

    // The next display cycle will force a game cycle
    displayCount = 0;

    // Total displayed frames for this cycle is one
    displayTotalCount = 1;

    // This value will be ignored by virtue of displayCount being 1
    aborted = FALSE;

    // Time it takes to do a display cycle
    displayTime = 30000;

    // Deviation in display time
    displayTimeDev = 0;

    // Networking

    // Next time networking will occur
    //LOG_DIAG(("Resetting network cycle to 2"))
    nextNetworkGameCycle = 2;

    // Muitiplayer games wait for the router to start proeedings
    stalled = TRUE;
    networkStallTime = timeFirstGameCycle;
    persistentStalls = FALSE;

    // Bump

    // Last time a bump occured in either direction
    lastPosBumpTime = Clock::Time::UsLwr() - BUMPINTERVALUS;
    lastNegBumpTime = Clock::Time::UsLwr() - BUMPINTERVALUS;

    // Number of positive bumps which have occured
    numPosBumps = 0;

    // Number of negative bumps which have occured
    numNegBumps = 0;

    // Pausing

    // Not paused
    paused = FALSE;
    stepOnce = FALSE;

    // Not benchmarking
    fastMode = FALSE;
    fastForwardMode = FALSE;
    displayMode = TRUE;

    // Performance status
    displaySkipRate = 0.0F;
    consecutiveAborts = 0;
    fallingBehind = FALSE;
    droppingFrames = FALSE;

    // Reset the mono display
    MonoBufWrite(monoBuffer,  1,  0, "Game Cycles", Mono::BRIGHT);
    MonoBufWrite(monoBuffer,  2,  0, "Cycle           -          ? cycles", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  3,  0, "Next Cycle Time -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  4,  0, "Current Time    -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  5,  0, "Offset Time     -          ? \xE6s", Mono::NORMAL);

    MonoBufWrite(monoBuffer,  7,  0, "Simulation Time", Mono::BRIGHT);
    MonoBufWrite(monoBuffer,  8,  0, "SimTime         -          ? s", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  9,  0, "TotalTime       -          ? s", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 10,  0, "Cycle           -          ? cycles", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 11,  0, "Paused          -          ? flag", Mono::NORMAL);

    MonoBufWrite(monoBuffer, 13,  0, "Display Information", Mono::BRIGHT);
    MonoBufWrite(monoBuffer, 14,  0, "Display Count   -          ? frames", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 15,  0, "Display Total   -          ? frames", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 16,  0, "Display Time    -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 17,  0, "Display Dev     -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 18,  0, "Aborted         -          ? flag", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 20,  0, "Interpolation   -            ", Mono::NORMAL);

    MonoBufWrite(monoBuffer, 21,  0, "Stalled Time    -            ", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 22,  0, "DisplayLag %    -            ", Mono::NORMAL);

    MonoBufWrite(monoBuffer,  1, 40, "Network Information", Mono::BRIGHT);
    MonoBufWrite(monoBuffer,  2, 40, "Next Cycle      -          ? cycles", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  3, 40, "Next Time       -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  4, 40, "Stalled         -          ? flag", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  5, 40, "Network Lag     -          ? ms", Mono::NORMAL);
    MonoBufWrite(monoBuffer,  6, 40, "ArrivaltoProc   -          ? ms", Mono::NORMAL);

    MonoBufWrite(monoBuffer, 10, 40, "Bumps", Mono::BRIGHT);
    MonoBufWrite(monoBuffer, 11, 40, "Last Pos Bump   -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 12, 40, "Num Pos Bumps   -          ? bumps", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 13, 40, "Last Neg Bump   -          ? \xE6s", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 14, 40, "Num Neg Bumps   -          ? bumps", Mono::NORMAL);

    MonoBufWrite(monoBuffer, 16, 40, "Synchronization", Mono::BRIGHT);
    MonoBufWrite(monoBuffer, 17, 40, "CRC             -          ?h", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 18, 40, "First Index     -          ?", Mono::NORMAL);
    MonoBufWrite(monoBuffer, 19, 40, "Next Index      -          ?", Mono::NORMAL);
  }


  //
  // Save
  //
  // Save system data
  //
  void Save(FScope *scope)
  {
    StdSave::TypeU32(scope, "SimCycle", simCycle);
  }


  //
  // Load
  //
  // Load system data
  //
  void Load(FScope *scope)
  {
    simCycle = StdLoad::TypeU32(scope, "SimCycle");
    simTotalTime = simCycle * INTERVAL;
  }


  //
  // DisplaySample:
  //
  // A sample of how long it takes to do a display cycle has been taken
  //
  void DisplaySample(U32 sample)
  {
    displayTimeDev = (displayTimeDev * 224 + abs(displayTime - sample) * 32) >> 8;
    displayTime = ((displayTime * 224 + sample * 32) >> 8) + 1;
  }


  //
  // GameCycleOffset: 
  //
  // Returns "interpolated" offset from last game cycle (in seconds)
  //
  F32 GameCycleOffset()
  {
    // In fast mode, game cycles advance each display frame
    if (fastMode || fastForwardMode)
    {
      return 0.0F;
    }

    // Update interpolated game cycle offset (in seconds) when 
    // aborted will not be TRUE
    S32 timeRemaining = timeNextGameCycle - Clock::Time::UsLwr();
    S32 offset = INTERVALUS - timeRemaining;
    F32 seconds = offset * 1e-6F; // = 1 / (1000000.0F);
    return (seconds);
  }


  //
  // TRUE if simulation is taking too long and displays are being forced
  //
  Bool IsFallingBehind()
  {
    return (fallingBehind);
  }


  //
  // TRUE if too many display frames are being dropped
  //
  Bool IsDroppingFrames()
  {
    return (droppingFrames);
  }


  //
  // TRUE if stalled
  //
  Bool IsStalled()
  {
    return (MultiPlayer::Data::Online() && persistentStalls);
  }


  //
  // Test: Test to see if we should proceed to the next game cycle
  //
  // To proceed to the next game cycle, one of the following must occur
  //
  // We complete the number of scheduled display cycles
  //  OR
  // The time to next cycle has expired
  //
  // If the time to next cycle has expired and we still have outstanding display
  // cycles to be done (this should happen rarely) then the aborted flag is set
  // to indicate to the game that it should NOT assume that the display information
  // is correct.
  //
  Bool Test()
  {
    ASSERT(initialized)

    U32 timeNow = Clock::Time::UsLwr();
    S32 timeRemaining = timeNextGameCycle - timeNow;

    MonoBufWriteV(monoBuffer, (14, 18, Mono::NORMAL, "%10d", displayCount));
    MonoBufWriteV(monoBuffer, (15, 18, Mono::NORMAL, "%10d", displayTotalCount));
    MonoBufWriteV(monoBuffer, (16, 18, Mono::NORMAL, "%10u", displayTime));
    MonoBufWriteV(monoBuffer, (17, 18, Mono::NORMAL, "%10u", displayTimeDev));
    MonoBufWriteV(monoBuffer, (18, 18, Mono::NORMAL, "%10d", aborted));

    #ifdef DEVELOPMENT
      // Display interpolation time
      char buf[12] = "[       ]";

      if (interpolationTime > 0.0F)
      {
        buf[1+S32(7.0F * interpolationTime * INTERVALINV)] = '\x10';
      }
      MonoBufWrite(monoBuffer, 20, 18, buf, Mono::NORMAL);
    #endif

    // Fast mode forces simulation always
    // Fast forward forces simulation only if not paused
    if (fastMode || (!paused && fastForwardMode))
    {
      // Dispatch orders
      Cycle();

      // Force simulation
      return (simCycle != 0);
    }

    displayCount--;

    // If the display counter has elapsed then 
    if (displayCount <= 0)
    {
      // DO NOT set displayCount to 0 here!!!
      //displayCount = 0;

      aborted = FALSE;
      return (Cycle());
    }
    else
    if (timeRemaining < 0)
    {
      aborted = TRUE;
      return (Cycle());    
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Compute: Calculate the number of display cycles we can perform in a game cycle
  //
  void Compute()
  {
    U32 timeNow = Clock::Time::UsLwr();
    S32 timeRemaining = timeNextGameCycle - timeNow;

    // Filter display lag towards 0 each tenth of a second
    if (displaySkipRate)
    {
      displaySkipRate = (displaySkipRate * Filter1);
    }

    // Force display in fast mode
    if (fastMode || fastForwardMode)
    {
      if (displayMode)
      {
        // Force a single display
        displayCount = 1;
        displayTotalCount = 1;
        aborted = FALSE;
      }
      else
      {
        // No displaying
        displayCount = 0;
        displayTotalCount = 0;
        aborted = TRUE;
      }

      // Keep next cycle up to date with current time
      timeNextGameCycle = timeNow + INTERVALUS;
    }
    else

    // If there is no time remaining then we cannot do any display
    if (timeRemaining < 0)
    {
      displayTotalCount = 0;
      displayCount = 0;
      aborted = TRUE;

      if (++consecutiveAborts >= 10)
      {
        // If 10 continuous simulation cycles have occured without a 
        // display then force a single display now
        consecutiveAborts = 0;
        fallingBehind = TRUE;

        displayTotalCount = 1;
        displayCount = 1;
        aborted = FALSE;
      }
    }
    else

    {
      displayTotalCount = timeRemaining / displayTime;
      displayCount = displayTotalCount;

      // A display cycle was 
      fallingBehind = FALSE;
      consecutiveAborts = 0;

      if (displayTotalCount == 0)
      {
        // If there is no time to do a display this cycle, filter the displaySkipRate variable towards 1
        displaySkipRate = displaySkipRate * Filter1 + Filter2;
      }
    }

    // Update the dropping frames flag every sim cycle
    if (droppingFrames)
    {
      // If rate falls below low threshold, turn flag off
      if (displaySkipRate < DISPLAY_SKIP_THRESHOLD_LO)
      {
        droppingFrames = FALSE;
      }
    }
    else
    {
      // If rate rises above high threshold, turn flag on
      if (displaySkipRate > DISPLAY_SKIP_THRESHOLD_HI)
      {
        droppingFrames = TRUE;
      }
    }

    MonoBufWriteV(monoBuffer, (22, 18, Mono::NORMAL, "%8.5f", displaySkipRate));
  }


  //
  // Cycle: Increment to the next game cycle
  //
  // out <- TRUE: proceed
  //        FALSE: wait
  //
  Bool Cycle(Bool force)
  {
    ASSERT(initialized)

    if (!MultiPlayer::Data::Online())
    {
      //
      // Single Player is straight forward
      //
      // Increment the game cycles
      // Add the current SimTime to SimTotalTime
      // SimTime for the next round is copied from the game speed
      //
      IncGameCycle();

      Orders::Dispatch();

      // Only increment simTotalTime if we're not paused
      if (!paused || stepOnce)
      {
        simCycle++;
        simTotalTime = simCycle * INTERVAL;
      }

      #ifndef MONO_DISABLED

      MonoBufWriteV(monoBuffer, ( 2, 18, Mono::NORMAL, "%10u", gameCycle));
      MonoBufWriteV(monoBuffer, ( 3, 18, Mono::NORMAL, "%10u", timeNextGameCycle));
      U32 time = Clock::Time::UsLwr();
      MonoBufWriteV(monoBuffer, ( 4, 18, Mono::NORMAL, "%10u", time));
      MonoBufWriteV(monoBuffer, ( 5, 18, Mono::NORMAL, "%10u", time - timeFirstGameCycle));
      MonoBufWriteV(monoBuffer, ( 8, 18, Mono::NORMAL, "%10.2f", INTERVAL));
      MonoBufWriteV(monoBuffer, ( 9, 18, Mono::NORMAL, "%10.1f", simTotalTime));
      MonoBufWriteV(monoBuffer, (10, 18, Mono::NORMAL, "%10u", simCycle));
      MonoBufWriteV(monoBuffer, (11, 18, Mono::NORMAL, "%10d", paused));

      #endif

      // If we're paused then the game should not proceed
      if (paused && !stepOnce)
      {
        Compute();
        return (FALSE);
      }
      if (stepOnce)
      {
        stepOnce = FALSE;
      }
      return (TRUE);
    }
    else
    {
      //
      // MultiPlayer is slightly more complex
      //
      // Are we stalled ?
      //   If so then ask the networking for the current network round
      //
      // Increment the game cycles
      // Have we reached the target network cycle
      // If we have ask the networking to insert vars for this network round
      // If the networking does not have this round yet then we stall
      //
      U32 nextTime;
      U32 arrivalTime;
      U32 nowTime = Clock::Time::UsLwr();

      if (force)
      {
        nextNetworkGameCycle = gameCycle;
      }

      if (stalled)
      {
        U32 period = nowTime - networkStallTime;

        // Have we crossed the threshold
        if (period > STALL_THRESHOLD)
        {
          persistentStalls = TRUE;
        }

        MonoBufWriteV(monoBuffer, (21, 18, Mono::NORMAL, "%8d", period / 1000));
      }
      else
      {
        if (persistentStalls)
        {
          // Turn off the stalled flag
          persistentStalls = FALSE;
        }
      }

      //LOG_DIAG(("gameCycle %d [nextNetworkCycle %d]", gameCycle, nextNetworkGameCycle))

      // Have we completed the required number of cycles ?
      if (gameCycle == nextNetworkGameCycle)
      {
        // Are we stalled ?
        if (stalled)
        {
          // Are we still stalled ?
          if (!MultiPlayer::Data::ProcessSyncData(networkSequence, nextTime, arrivalTime))
          {
            return (FALSE);
          }

          // Unstalled
          stalled = FALSE;

          // Work out how long we were stalled for
          U32 period = nowTime - networkStallTime;

          // Clamp period to one full game cycle
          period = Min(period, INTERVALUS);

          // Move game time by this amount to prevent future stalls
          timeNextGameCycle += period;
          //LOG_DIAG(("Stall forced adjustment of %d", period))
        }
        else
        {
          MultiPlayer::Data::ProcessSyncData(networkSequence, nextTime, arrivalTime);
          //LOG_DIAG(("!Stalled: Next time %d", nextTime))
        }

        // Are we lagged ?
        if (MultiPlayer::Data::GetLag())
        {
          // If we're allowed to bump, then do so
          if (nowTime - lastPosBumpTime > BUMPINTERVALUS)
          {
            //LOG_DIAG(("Negative Bump : %d - %d (behind)", timeNextGameCycle, BUMPTIMENEGUS))

            timeNextGameCycle -= BUMPTIMENEGUS;
            lastNegBumpTime = nowTime;
            numNegBumps++;
            MonoBufWriteV(monoBuffer, (13, 58, Mono::NORMAL, "%10u", lastNegBumpTime));
            MonoBufWriteV(monoBuffer, (14, 58, Mono::NORMAL, "%10u", numNegBumps));
          }
        }

        // Did we received the network round ?
        if (nextTime)
        {
          // Calculate the next game cycle which networking will occur on
          nextNetworkGameCycle = gameCycle + (nextTime / 100);
          //LOG_DIAG(("Setting next network cycle to %d", nextNetworkGameCycle))

          //U32 now = nowTime; 
          //LOG_DIAG(("Processing Sync Data @ %d which arived @ %d (diff %d)", now, arrivalTime, now - arrivalTime))

          // Update mono
          MonoBufWriteV(monoBuffer, ( 2, 58, Mono::NORMAL, "%10u", nextNetworkGameCycle));
          MonoBufWriteV(monoBuffer, ( 3, 58, Mono::NORMAL, "%10u", nextTime));
          MonoBufWriteV(monoBuffer, ( 4, 58, Mono::NORMAL, "%10u", stalled));
          MonoBufWriteV(monoBuffer, ( 5, 58, Mono::NORMAL, "%10u", MultiPlayer::Data::GetLag()));
          MonoBufWriteV(monoBuffer, ( 6, 58, Mono::NORMAL, "%10u", nowTime - arrivalTime));
        }
        else
        {
          // Stall for more time
          stalled = TRUE;
          networkStallTime = Clock::Time::UsLwr();

          // If we're allowed to bump, then do so
          if (networkStallTime - lastNegBumpTime > BUMPINTERVALUS)
          {
            //LOG_DIAG(("Positive Bump : %d + %d (infront)", timeNextGameCycle, BUMPTIMEPOSUS))

            timeNextGameCycle += BUMPTIMEPOSUS;
            lastPosBumpTime = networkStallTime;
            numPosBumps++;
            MonoBufWriteV(monoBuffer, (11, 58, Mono::NORMAL, "%10u", lastPosBumpTime));
            MonoBufWriteV(monoBuffer, (12, 58, Mono::NORMAL, "%10u", numPosBumps));
          }

          // Update mono
          MonoBufWriteV(monoBuffer, ( 4, 58, Mono::NORMAL, "%10u", stalled));

          // The game cannot proceed
          return (FALSE);
        }
      }

      // Perform the cycle
      IncGameCycle();

      // Order dispatch
      Orders::Dispatch();

      // Only increment simTotalTime if we're not paused
      if (!paused || stepOnce)
      {
        simCycle++;
        simTotalTime = simCycle * INTERVAL;
      }

      // Update mono
      #ifndef MONO_DISABLED

      MonoBufWriteV(monoBuffer, ( 2, 18, Mono::NORMAL, "%10u", gameCycle));
      MonoBufWriteV(monoBuffer, ( 3, 18, Mono::NORMAL, "%10u", timeNextGameCycle));
      U32 time = Clock::Time::UsLwr();
      MonoBufWriteV(monoBuffer, ( 4, 18, Mono::NORMAL, "%10u", time));
      MonoBufWriteV(monoBuffer, ( 5, 18, Mono::NORMAL, "%10u", time - timeFirstGameCycle));
      MonoBufWriteV(monoBuffer, ( 8, 18, Mono::NORMAL, "%10.2f", INTERVAL));
      MonoBufWriteV(monoBuffer, ( 9, 18, Mono::NORMAL, "%10.1f", simTotalTime));
      MonoBufWriteV(monoBuffer, (10, 18, Mono::NORMAL, "%10u", simCycle));
      MonoBufWriteV(monoBuffer, (11, 18, Mono::NORMAL, "%10u", paused));

      #endif

      // If we're paused then the game should not proceed
      if (paused && !stepOnce)
      {
        Compute();
        return (FALSE);
      }
      if (stepOnce)
      {
        stepOnce = FALSE;
      }
      return (TRUE);
    }
  }


  //
  // Pause: Pause/Unpause the game
  //
  void Pause(Bool triggerMessage)
  {
    if (paused)
    {
      //LOG_DIAG(("Unpausing [%f]", simTotalTime))
      if (triggerMessage)
      {
        Message::TriggerGameMessage(0x1BF1D0DD); // "Game::Unpaused"
      }

      // Unpause
      paused = FALSE;
    }
    else
    {
      //LOG_DIAG(("Pausing [%f]", simTotalTime))
      if (triggerMessage)
      {
        Message::TriggerGameMessage(0xA2A95F95); // "Game::Paused"
      }

      // Pause
      paused = TRUE;
    }
  }


  //
  // Return the current paused state
  //
  Bool Paused()
  {
    return (paused);
  }


  //
  // StepOnce: Single Step the game
  //
  void StepOnce()
  {
    //LOG_DIAG(("Stepping Once [%f]", simTotalTime))
    Message::TriggerGameMessage(0x80014D42); // "Game::StepOnce"

    stepOnce = TRUE;
    paused = TRUE;
  }


  //
  // Readjust: Adjust gametime back to real time, after unpausing a single player game
  //
  void Readjust()
  {
    if (!MultiPlayer::Data::Online())
    {
      // Keep next cycle up to date with current time
      timeNextGameCycle = Clock::Time::UsLwr() + INTERVALUS;
    }

    // Force a single display
    displayCount = 1;
    displayTotalCount = 1;
    aborted = FALSE;
  }


  //
  // Sync: Synchronize network cycle with game cycle - used for multiplayer joins
  //
  void Synchronize()
  {
    nextNetworkGameCycle = gameCycle + 1;
    //LOG_DIAG(("Synchronizing network cycle [%d]", nextNetworkGameCycle))
  }


  //
  // SetFastMode: Enable/disable fast simulation
  //
  void SetFastMode(Bool flag)
  {
    fastMode = flag;
    //LOG_DIAG(("GameTime::FastMode %s", fastMode ? "on" : "off"))
  }


  //
  // GetFastMode: Return current fast mode flag
  //
  Bool GetFastMode()
  {
    return (fastMode);
  }

  
  //
  // GetFastForwardMode: Return fast forward flag
  //
  Bool GetFastForwardMode()
  {
    return (fastForwardMode);
  }


  //
  // SetDisplayMode: Enable/disable display
  //
  void SetDisplayMode(Bool flag)
  {
    displayMode = flag;
    //LOG_DIAG(("GameTime::DisplayMode %s", displayMode ? "on" : "off"))
  }


  //
  // GetDisplayMode: Return current display mode flag
  //
  Bool GetDisplayMode()
  {
    return (displayMode);
  }


  //
  // TimeSinceCycle
  //
  // Returns the number of seconds since the given sim cycle
  //
  F32 TimeSinceCycle(U32 cycle)
  {
    return (F32(simCycle - cycle) * SimTime());
  }


  //
  // IncGameCycle: Increment the game cycle
  //
  void IncGameCycle()
  {
    SYNC(simCycle)

    Sync::Save();

    // Write the current CRC to the mono
    MonoBufWriteV(monoBuffer, (17, 59, Mono::NORMAL, " %08X", Sync::GetSavedCrc()));
    MonoBufWriteV(monoBuffer, (18, 58, Mono::NORMAL, "%10u", Sync::IndexFirst()));
    MonoBufWriteV(monoBuffer, (19, 58, Mono::NORMAL, "%10u", Sync::IndexNext()));
 
    // Roll on to the next game cycle
    gameCycle++;

    timeNextGameCycle += INTERVALUS;
  }


  //
  // CmdHandler: Command handler for the game timing system
  //
  void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized)

    switch (pathCrc)
    {
      case 0x453EA9D4: // "timing.reset"
        Reset();
        break;

      case 0xD778D492: // "timing.report"
        CON_DIAG(("Game Cycles"))
        CON_DIAG(("Cycle           - %10u", gameCycle))
        CON_DIAG(("Next Cycle Time - %10u", timeNextGameCycle))
        CON_DIAG(("Current Time    - %10u", Clock::Time::UsLwr()))
        CON_DIAG(("Simulation Time - %10.2f", INTERVAL))
        CON_DIAG(("SimTotalTime    - %10.1f", simTotalTime))
        CON_DIAG(("Sim Cycle       - %10u", simCycle))
        break;

      case 0x125523AB: // "timing.pause"
        if (MultiPlayer::Data::Online())
        {
          CON_ERR(("This will get the game out of sync ... use client.pause instead"))
        }
        else
        {
          Pause();
        }
        break;

      case 0x155197DE: // "timing.steponce"
        if (MultiPlayer::Data::Online())
        {
          CON_ERR(("This will get the game out of sync ... use client.steponce instead"))
        }
        else
        {
          StepOnce();
        }
        break;

      case 0xC16CBE01: // "timing.readjust"
        Readjust();
        break;

      case 0xF263B615: // "timing.fastforward"
      {
        if (!MultiPlayer::Data::Online())
        {
          fastForwardMode = *fastForward;
        }
        break;
      }

      case 0x74AB3301: // "timing.fastdisplay"
      {
        SetDisplayMode(!*fastDisplay);
        break;
      }

      default:
        ERR_FATAL(("Unknown timing var %08x", pathCrc));
        break;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  // 
  // Class Timer 
  //

  //
  // Timer::Start
  //
  // Set the timer to go off in "time" seconds
  //
  void Timer::Start(F32 time)
  {
    // Calculate the trigger time
    triggerTime = U32(simCycle + (time * SimTimeInv()));
  }


  //
  // Timer::Test
  // 
  // Return TRUE if the timer has elapsed
  //
  Bool Timer::Test()
  {
    return ((simCycle >= triggerTime) ? TRUE : FALSE);
  }


  //
  // Timer::GetRemaining
  //
  // Time remaining
  //
  F32 Timer::GetRemaining() const
  {
    return (F32(triggerTime - simCycle) * SimTime());
  }


  //
  // Timer::Reset
  //
  // Reset the timer
  //
  void Timer::Reset()
  {
    triggerTime = 0;
  }


  //
  // Timer::SaveState
  //
  void Timer::SaveState(FScope *fScope)
  {
    fScope->AddArgInteger(triggerTime);
  }


  //
  // Timer::LoadState
  //
  void Timer::LoadState(FScope *fScope)
  {
    triggerTime = fScope->NextArgInteger();
  }
}
