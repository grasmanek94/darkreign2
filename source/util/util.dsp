# Microsoft Developer Studio Project File - Name="util" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=util - Win32 Development
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "util.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "util.mak" CFG="util - Win32 Development"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "util - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "util - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "util - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/util", IDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "util - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/util"
# PROP Intermediate_Dir "../Release/util"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "util - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/util"
# PROP Intermediate_Dir "../Debug/util"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "util - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "util___Win32_Development"
# PROP BASE Intermediate_Dir "util___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/util"
# PROP Intermediate_Dir "../Development/util"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"std.h" /FD /c
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

# Name "util - Win32 Release"
# Name "util - Win32 Debug"
# Name "util - Win32 Development"
# Begin Group "var"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\varfile.cpp
# End Source File
# Begin Source File

SOURCE=.\varfile.h
# End Source File
# Begin Source File

SOURCE=.\varitem.cpp
# End Source File
# Begin Source File

SOURCE=.\varptr.cpp
# End Source File
# Begin Source File

SOURCE=.\varscope.cpp
# End Source File
# Begin Source File

SOURCE=.\varsys.cpp
# End Source File
# Begin Source File

SOURCE=.\varsys.h
# End Source File
# End Group
# Begin Group "types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\area.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\mathtypes.cpp
# End Source File
# Begin Source File

SOURCE=.\mathtypes.h
# End Source File
# Begin Source File

SOURCE=.\utiltypes.cpp
# End Source File
# Begin Source File

SOURCE=.\utiltypes.h
# End Source File
# End Group
# Begin Group "multilanguage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\babel.cpp
# End Source File
# Begin Source File

SOURCE=.\babel.h
# End Source File
# Begin Source File

SOURCE=.\multilanguage.cpp
# End Source File
# Begin Source File

SOURCE=.\multilanguage.h
# End Source File
# End Group
# Begin Group "operators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arithmeticoperator.h
# End Source File
# Begin Source File

SOURCE=.\comparison.cpp
# End Source File
# Begin Source File

SOURCE=.\comparison.h
# End Source File
# Begin Source File

SOURCE=.\operation.cpp
# End Source File
# Begin Source File

SOURCE=.\operation.h
# End Source File
# Begin Source File

SOURCE=.\relationaloperator.cpp
# End Source File
# Begin Source File

SOURCE=.\relationaloperator.h
# End Source File
# Begin Source File

SOURCE=.\switch.cpp
# End Source File
# Begin Source File

SOURCE=.\switch.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\balancetable.h
# End Source File
# Begin Source File

SOURCE=.\bitarray.h
# End Source File
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\event.cpp
# End Source File
# Begin Source File

SOURCE=.\event.h
# End Source File
# Begin Source File

SOURCE=.\lopassfilter.h
# End Source File
# Begin Source File

SOURCE=.\midpointline.h
# End Source File
# Begin Source File

SOURCE=.\param.cpp
# End Source File
# Begin Source File

SOURCE=.\param.h
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "util - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "util - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "util - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\perfstats.cpp
# End Source File
# Begin Source File

SOURCE=.\perfstats.h
# End Source File
# Begin Source File

SOURCE=.\plasma.cpp
# End Source File
# Begin Source File

SOURCE=.\plasma.h
# End Source File
# Begin Source File

SOURCE=.\promotelink.h
# End Source File
# Begin Source File

SOURCE=..\coregame\spline.cpp
# End Source File
# Begin Source File

SOURCE=..\coregame\spline.h
# End Source File
# Begin Source File

SOURCE=.\statemachine.h
# End Source File
# Begin Source File

SOURCE=.\stats.cpp
# End Source File
# Begin Source File

SOURCE=.\stats.h
# End Source File
# Begin Source File

SOURCE=.\stdload.cpp
# End Source File
# Begin Source File

SOURCE=.\stdload.h
# End Source File
# Begin Source File

SOURCE=.\sunpos.cpp
# End Source File
# Begin Source File

SOURCE=.\sunpos.h
# End Source File
# Begin Source File

SOURCE=.\tracksys.cpp
# End Source File
# Begin Source File

SOURCE=.\tracksys.h
# End Source File
# End Target
# End Project
