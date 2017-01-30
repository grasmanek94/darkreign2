# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=game - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "game - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/game", DNAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/game"
# PROP Intermediate_Dir "../Release/game"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/game"
# PROP Intermediate_Dir "../Debug/game"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_Development"
# PROP BASE Intermediate_Dir "game___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/game"
# PROP Intermediate_Dir "../Development/game"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"std.h" /FD /c
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

# Name "game - Win32 Release"
# Name "game - Win32 Debug"
# Name "game - Win32 Development"
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\game_addonlist.cpp
# End Source File
# Begin Source File

SOURCE=.\game_addonlist.h
# End Source File
# Begin Source File

SOURCE=.\game_campaignselection.cpp
# End Source File
# Begin Source File

SOURCE=.\game_campaignselection.h
# End Source File
# Begin Source File

SOURCE=.\game_difficultylist.cpp
# End Source File
# Begin Source File

SOURCE=.\game_difficultylist.h
# End Source File
# Begin Source File

SOURCE=.\game_login.cpp
# End Source File
# Begin Source File

SOURCE=.\game_login.h
# End Source File
# Begin Source File

SOURCE=.\game_missionselection.cpp
# End Source File
# Begin Source File

SOURCE=.\game_missionselection.h
# End Source File
# Begin Source File

SOURCE=.\game_saveload.cpp
# End Source File
# Begin Source File

SOURCE=.\game_saveload.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\campaigns.cpp
# End Source File
# Begin Source File

SOURCE=.\campaigns.h
# End Source File
# Begin Source File

SOURCE=.\demo.cpp
# End Source File
# Begin Source File

SOURCE=.\demo.h
# End Source File
# Begin Source File

SOURCE=.\difficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\difficulty.h
# End Source File
# Begin Source File

SOURCE=.\gamegod.cpp
# End Source File
# Begin Source File

SOURCE=.\gamegod.h
# End Source File
# Begin Source File

SOURCE=.\gameruncodes.cpp
# End Source File
# Begin Source File

SOURCE=.\gameruncodes.h
# End Source File
# Begin Source File

SOURCE=.\gamesound.cpp
# End Source File
# Begin Source File

SOURCE=.\gamesound.h
# End Source File
# Begin Source File

SOURCE=.\gametime.cpp
# End Source File
# Begin Source File

SOURCE=.\gametime.h
# End Source File
# Begin Source File

SOURCE=.\guid.cpp
# End Source File
# Begin Source File

SOURCE=.\guid.h
# End Source File
# Begin Source File

SOURCE=.\missions.cpp
# End Source File
# Begin Source File

SOURCE=.\missions.h
# End Source File
# Begin Source File

SOURCE=.\mods.cpp
# End Source File
# Begin Source File

SOURCE=.\mods.h
# End Source File
# Begin Source File

SOURCE=.\movieplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\movieplayer.h
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "game - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\savegame.cpp
# End Source File
# Begin Source File

SOURCE=.\savegame.h
# End Source File
# Begin Source File

SOURCE=.\sides.cpp
# End Source File
# Begin Source File

SOURCE=.\sides.h
# End Source File
# Begin Source File

SOURCE=.\sync.cpp
# End Source File
# Begin Source File

SOURCE=.\sync.h
# End Source File
# Begin Source File

SOURCE=.\trackplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\trackplayer.h
# End Source File
# Begin Source File

SOURCE=.\user.cpp
# End Source File
# Begin Source File

SOURCE=.\user.h
# End Source File
# Begin Source File

SOURCE=.\win32reg.cpp
# End Source File
# Begin Source File

SOURCE=.\win32reg.h
# End Source File
# End Target
# End Project
