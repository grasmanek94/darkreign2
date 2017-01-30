/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    TTTTT  IIIII  M   M  EEEEE  RRRR                         %
%                      T      I    MM MM  E      R   R                        %
%                      T      I    M M M  EEE    RRRR                         %
%                      T      I    M   M  E      R R                          %
%                      T    IIIII  M   M  EEEEE  R  R                         %
%                                                                             %
%                                                                             %
%                        ImageMagick Timing Methods                           %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright (C) 2000 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Contributed by Bill Radcliffe and Bob Friesenhahn.
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

#if defined(macintosh)
#define CLK_TCK  CLOCKS_PER_SEC
#endif
#if !defined(CLK_TCK)
#define CLK_TCK  sysconf(_SC_CLK_TCK)
#endif

/*
  Forward declarations.
*/
static double
  UserTime(void);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n t i n u e T i m e r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ContinueTimer resumes a stopped stopwatch. The stopwatch continues
%  counting from the last StartTimer() onwards.
%
%  The format of the ContinueTimer method is:
%
%      unsigned int ContinueTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Time statistics structure.
%
*/
Export unsigned int ContinueTimer(TimerInfo *time_info)
{
  if (time_info->state == UndefinedTimerState)
    return(False);
  if (time_info->state == StoppedTimerState)
    {
      time_info->user.total-=time_info->user.stop-time_info->user.start;
      time_info->elapsed.total-=
        time_info->elapsed.stop-time_info->elapsed.start;
    }
  time_info->state=RunningTimerState;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E l a p s e d T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ElapsedTime returns the elapsed time (in seconds) since the last
%  call to StartTimer().
%
%  The format of the ElapsedTime method is:
%
%      double ElapsedTime()
%
*/
static double ElapsedTime(void)
{
#if !defined(vms) && !defined(macintosh) && !defined(WIN32)
  struct tms
    timer;

  return((double) times(&timer)/CLK_TCK);
#else
#if defined(WIN32)
  return(NTElapsedTime());
#else
  return((double) clock()/CLK_TCK);
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E l a p s e d T i m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetElapsedTime returns the elapsed time (in seconds) passed between
%  the start and stop events. If the stopwatch is still running, it is stopped
%  first.
%
%  The format of the GetElapsedTime method is:
%
%      double GetElapsedTime(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
Export double GetElapsedTime(TimerInfo *time_info)
{
  if (time_info->state == UndefinedTimerState)
    return(0.0);
  if (time_info->state == RunningTimerState)
    StopTimer(time_info);
  return(time_info->elapsed.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t T i m e r I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetTimerInfo initializes the TimerInfo structure.
%
%  The format of the GetTimerInfo method is:
%
%      void GetTimerInfo(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
Export void GetTimerInfo(TimerInfo *time_info)
{
  /*
    Create a stopwatch and start it.
  */
  time_info->state=UndefinedTimerState;
  time_info->user.total=0;
  time_info->elapsed.total=0;
  StartTimer(time_info,True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t U s e r T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetUserTime returns the User time (user and system) by the operating
%  system (in seconds) between the start and stop events. If the stopwatch is
%  still running, it is stopped first.
%
%  The format of the GetUserTime method is:
%
%      double GetUserTime(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
Export double GetUserTime(TimerInfo *time_info)
{
  if (time_info->state == UndefinedTimerState)
    return(0.0);
  if (time_info->state == RunningTimerState)
    StopTimer(time_info);
  return(time_info->user.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ResetTimer resets the stopwatch.
%
%  The format of the ResetTimer method is:
%
%      void ResetTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
Export void ResetTimer(TimerInfo *time_info)
{
  StopTimer(time_info);
  time_info->elapsed.stop=0.0;
  time_info->user.stop=0.0;  
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t a r t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method StartTimer starts the stopwatch.
%
%  The format of the StartTimer method is:
%
%      void StartTimer(TimerInfo *time_info,const unsigned int reset)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
%    o  reset: If reset is True, then the stopwatch is reset prior to starting.
%       If reset is False, then timing is continued without resetting the
%       stopwatch.
%
*/
Export void StartTimer(TimerInfo *time_info, const unsigned int reset)
{
  if (reset)
    {
      /*
        Reset the stopwatch before starting it.
      */
      time_info->user.total=0;
      time_info->elapsed.total=0;
    }
  if (time_info->state != RunningTimerState)
    {
      time_info->elapsed.start=ElapsedTime();
      time_info->user.start=UserTime();
    }
  time_info->state=RunningTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t o p T i m e r                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method StopTimer stops the stopwatch.
%
%  The format of the StopTimer method is:
%
%      void StopTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
Export void StopTimer(TimerInfo *time_info)
{
  time_info->elapsed.stop=ElapsedTime();
  time_info->user.stop=UserTime();
  if (time_info->state == RunningTimerState)
    {
      time_info->user.total+=time_info->user.stop-time_info->user.start;
      time_info->elapsed.total+=
        time_info->elapsed.stop-time_info->elapsed.start;
    }
  time_info->state=StoppedTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   U s e r T i m e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UserTime returns the total time the process has been scheduled (in
%  seconds) since the last call to StartTimer().
%
%  The format of the UserTime method is:
%
%      double UserTime()
%
*/
static double UserTime(void)
{
#if !defined(vms) && !defined(macintosh) && !defined(WIN32)
  struct tms
    timer;

  times(&timer);
  return((double) (timer.tms_utime+timer.tms_stime)/CLK_TCK);
#else
#if defined(WIN32)
  return(NTUserTime());
#else
  return((double) clock()/CLK_TCK);
#endif
#endif
}
