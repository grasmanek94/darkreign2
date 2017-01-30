# Microsoft Developer Studio Project File - Name="coregame_tasks" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_tasks - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_tasks.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_tasks.mak" CFG="coregame_tasks - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_tasks - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_tasks - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_tasks - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_tasks", ZKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_tasks - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_tasks"
# PROP Intermediate_Dir "../Release/coregame_tasks"
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

!ELSEIF  "$(CFG)" == "coregame_tasks - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_tasks"
# PROP Intermediate_Dir "../Debug/coregame_tasks"
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

!ELSEIF  "$(CFG)" == "coregame_tasks - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_tasks___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_tasks___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_tasks"
# PROP Intermediate_Dir "../Development/coregame_tasks"
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

# Name "coregame_tasks - Win32 Release"
# Name "coregame_tasks - Win32 Debug"
# Name "coregame_tasks - Win32 Development"
# Begin Group "advanced tasks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tasks_offmapidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_offmapidle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_parasite.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_parasitebomb.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_parasitebomb.h
# End Source File
# Begin Source File

SOURCE=.\tasks_parasitesight.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_parasitesight.h
# End Source File
# Begin Source File

SOURCE=.\tasks_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_resourceregen.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_resourceregen.h
# End Source File
# Begin Source File

SOURCE=.\tasks_restore.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_restoremobile.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_restoremobile.h
# End Source File
# Begin Source File

SOURCE=.\tasks_restorestatic.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_restorestatic.h
# End Source File
# Begin Source File

SOURCE=.\tasks_spyidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_spyidle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_transport.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_transportpad.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_transportpad.h
# End Source File
# Begin Source File

SOURCE=.\tasks_transportunload.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_transportunload.h
# End Source File
# Begin Source File

SOURCE=.\tasks_trapidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_trapidle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_wallidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_wallidle.h
# End Source File
# End Group
# Begin Group "map tasks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tasks_map.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_mapdeath.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_mapdeath.h
# End Source File
# Begin Source File

SOURCE=.\tasks_mapidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_mapidle.h
# End Source File
# End Group
# Begin Group "squad tasks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tasks_squad.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadattack.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadattack.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadboard.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadboard.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadexplore.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadexplore.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadfollowtag.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadfollowtag.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadformation.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadformation.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadguard.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadguard.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadmove.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadmove.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadmovetogether.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadmovetogether.h
# End Source File
# Begin Source File

SOURCE=.\tasks_squadrestore.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_squadrestore.h
# End Source File
# End Group
# Begin Group "unit tasks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tasks_unit.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitanimate.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitanimate.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitattack.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitattack.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitboard.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitboard.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitbuild.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitbuild.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitcloak.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitcloak.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitcollect.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitcollect.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitconstruct.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitconstruct.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitconstructor.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitconstructor.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitexplore.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitexplore.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitfire.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitfire.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitgorestore.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitgorestore.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitguard.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitguard.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitidle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitidle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitmove.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitmove.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitmoveover.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitmoveover.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitpowerdown.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitpowerdown.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitrecycle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitrecycle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitrefinery.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitrefinery.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitsquadmiddle.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitsquadmiddle.h
# End Source File
# Begin Source File

SOURCE=.\tasks_unitupgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\tasks_unitupgrade.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_coregame.h"
# End Source File
# Begin Source File

SOURCE=..\coregame\stdinc_coregame.h
# End Source File
# Begin Source File

SOURCE=.\task.h
# End Source File
# Begin Source File

SOURCE=.\taskctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\taskctrl.h
# End Source File
# Begin Source File

SOURCE=.\taskutil.cpp
# End Source File
# Begin Source File

SOURCE=.\taskutil.h
# End Source File
# End Target
# End Project
