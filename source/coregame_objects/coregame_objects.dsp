# Microsoft Developer Studio Project File - Name="coregame_objects" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_objects - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_objects.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_objects.mak" CFG="coregame_objects - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_objects - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_objects - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_objects - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_objects", WKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_objects - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_objects"
# PROP Intermediate_Dir "../Release/coregame_objects"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"stdinc_coregame.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "coregame_objects - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_objects"
# PROP Intermediate_Dir "../Debug/coregame_objects"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "DEVELOPMENT" /Yu"stdinc_coregame.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "coregame_objects - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_objects___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_objects___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_objects"
# PROP Intermediate_Dir "../Development/coregame_objects"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"stdinc_coregame.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"stdinc_coregame.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "coregame_objects - Win32 Release"
# Name "coregame_objects - Win32 Debug"
# Name "coregame_objects - Win32 Development"
# Begin Group "game objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\gameobj.h
# End Source File
# Begin Source File

SOURCE=.\gameobjctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\gameobjctrl.h
# End Source File
# Begin Source File

SOURCE=.\gameobjdec.h
# End Source File
# Begin Source File

SOURCE=.\objdec.h
# End Source File
# End Group
# Begin Group "map objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mapobj.cpp
# End Source File
# Begin Source File

SOURCE=.\mapobj.h
# End Source File
# Begin Source File

SOURCE=.\mapobjctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\mapobjctrl.h
# End Source File
# Begin Source File

SOURCE=.\mapobjdec.h
# End Source File
# Begin Source File

SOURCE=.\mapobjiter.cpp
# End Source File
# Begin Source File

SOURCE=.\mapobjiter.h
# End Source File
# Begin Source File

SOURCE=.\mapobjlist.h
# End Source File
# Begin Source File

SOURCE=.\mapobjouchlist.h
# End Source File
# Begin Source File

SOURCE=.\propobj.cpp
# End Source File
# Begin Source File

SOURCE=.\propobj.h
# End Source File
# End Group
# Begin Group "misc objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bookmarkobj.cpp
# End Source File
# Begin Source File

SOURCE=.\bookmarkobj.h
# End Source File
# Begin Source File

SOURCE=.\bookmarkobjdec.h
# End Source File
# Begin Source File

SOURCE=.\markerobj.cpp
# End Source File
# Begin Source File

SOURCE=.\markerobj.h
# End Source File
# Begin Source File

SOURCE=.\markerobjdec.h
# End Source File
# Begin Source File

SOURCE=.\regionobj.cpp
# End Source File
# Begin Source File

SOURCE=.\regionobj.h
# End Source File
# Begin Source File

SOURCE=.\regionobjdec.h
# End Source File
# Begin Source File

SOURCE=.\tagobj.cpp
# End Source File
# Begin Source File

SOURCE=.\tagobj.h
# End Source File
# Begin Source File

SOURCE=.\tagobjdec.h
# End Source File
# Begin Source File

SOURCE=.\trailobj.cpp
# End Source File
# Begin Source File

SOURCE=.\trailobj.h
# End Source File
# Begin Source File

SOURCE=.\trailobjdec.h
# End Source File
# End Group
# Begin Group "offmap objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\offmapbombobj.cpp
# End Source File
# Begin Source File

SOURCE=.\offmapbombobj.h
# End Source File
# Begin Source File

SOURCE=.\offmapobj.cpp
# End Source File
# Begin Source File

SOURCE=.\offmapobj.h
# End Source File
# Begin Source File

SOURCE=.\offmapobjdec.h
# End Source File
# Begin Source File

SOURCE=.\offmapspawnobj.cpp
# End Source File
# Begin Source File

SOURCE=.\offmapspawnobj.h
# End Source File
# Begin Source File

SOURCE=.\offmapstrikeobj.cpp
# End Source File
# Begin Source File

SOURCE=.\offmapstrikeobj.h
# End Source File
# Begin Source File

SOURCE=.\offmapteamobj.cpp
# End Source File
# Begin Source File

SOURCE=.\offmapteamobj.h
# End Source File
# End Group
# Begin Group "resource objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resourceobj.cpp
# End Source File
# Begin Source File

SOURCE=.\resourceobj.h
# End Source File
# Begin Source File

SOURCE=.\resourceobjdec.h
# End Source File
# Begin Source File

SOURCE=.\resourceobjiter.cpp
# End Source File
# Begin Source File

SOURCE=.\resourceobjiter.h
# End Source File
# End Group
# Begin Group "squad objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\squadobj.cpp
# End Source File
# Begin Source File

SOURCE=.\squadobj.h
# End Source File
# Begin Source File

SOURCE=.\squadobjctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\squadobjctrl.h
# End Source File
# Begin Source File

SOURCE=.\squadobjdec.h
# End Source File
# End Group
# Begin Group "unit objects"

# PROP Default_Filter ""
# Begin Group "advanced unit objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\parasiteobj.cpp
# End Source File
# Begin Source File

SOURCE=.\parasiteobj.h
# End Source File
# Begin Source File

SOURCE=.\restoreobj.cpp
# End Source File
# Begin Source File

SOURCE=.\restoreobj.h
# End Source File
# Begin Source File

SOURCE=.\restoreobjdec.h
# End Source File
# Begin Source File

SOURCE=.\spyobj.cpp
# End Source File
# Begin Source File

SOURCE=.\spyobj.h
# End Source File
# Begin Source File

SOURCE=.\spyobjdec.h
# End Source File
# Begin Source File

SOURCE=.\transportobj.cpp
# End Source File
# Begin Source File

SOURCE=.\transportobj.h
# End Source File
# Begin Source File

SOURCE=.\trapobj.cpp
# End Source File
# Begin Source File

SOURCE=.\trapobj.h
# End Source File
# Begin Source File

SOURCE=.\wallobj.cpp
# End Source File
# Begin Source File

SOURCE=.\wallobj.h
# End Source File
# Begin Source File

SOURCE=.\wallobjdec.h
# End Source File
# End Group
# Begin Group "base unit objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\unitevacuate.cpp
# End Source File
# Begin Source File

SOURCE=.\unitevacuate.h
# End Source File
# Begin Source File

SOURCE=.\unitobj.cpp
# End Source File
# Begin Source File

SOURCE=.\unitobj.h
# End Source File
# Begin Source File

SOURCE=.\unitobjctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\unitobjctrl.h
# End Source File
# Begin Source File

SOURCE=.\unitobjdec.h
# End Source File
# Begin Source File

SOURCE=.\unitobjfinder.cpp
# End Source File
# Begin Source File

SOURCE=.\unitobjfinder.h
# End Source File
# Begin Source File

SOURCE=.\unitobjinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\unitobjinfo.h
# End Source File
# Begin Source File

SOURCE=.\unitobjiter.cpp
# End Source File
# Begin Source File

SOURCE=.\unitobjiter.h
# End Source File
# Begin Source File

SOURCE=.\unitobjlist.h
# End Source File
# End Group
# End Group
# Begin Group "weapon objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\deliveryprojectileobj.cpp
# End Source File
# Begin Source File

SOURCE=.\deliveryprojectileobj.h
# End Source File
# Begin Source File

SOURCE=.\explosionobj.cpp
# End Source File
# Begin Source File

SOURCE=.\explosionobj.h
# End Source File
# Begin Source File

SOURCE=.\explosionobjdec.h
# End Source File
# Begin Source File

SOURCE=.\projectileobj.cpp
# End Source File
# Begin Source File

SOURCE=.\projectileobj.h
# End Source File
# Begin Source File

SOURCE=.\projectileobjdec.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "coregame_objects - Win32 Release"

# ADD CPP /Yc"stdinc_coregame.h"

!ELSEIF  "$(CFG)" == "coregame_objects - Win32 Debug"

# ADD CPP /Yc"stdinc_coregame.h"

!ELSEIF  "$(CFG)" == "coregame_objects - Win32 Development"

# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\coregame\stdinc_coregame.h
# End Source File
# End Target
# End Project
