# Microsoft Developer Studio Project File - Name="coregame_particles" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_particles - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_particles.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_particles.mak" CFG="coregame_particles - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_particles - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_particles - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_particles - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_particles", YKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_particles - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_particles"
# PROP Intermediate_Dir "../Release/coregame_particles"
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

!ELSEIF  "$(CFG)" == "coregame_particles - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_particles"
# PROP Intermediate_Dir "../Debug/coregame_particles"
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

!ELSEIF  "$(CFG)" == "coregame_particles - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_particles___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_particles___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_particles"
# PROP Intermediate_Dir "../Development/coregame_particles"
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

# Name "coregame_particles - Win32 Release"
# Name "coregame_particles - Win32 Debug"
# Name "coregame_particles - Win32 Development"
# Begin Source File

SOURCE=.\beamrender_base.cpp
# End Source File
# Begin Source File

SOURCE=.\beamrender_base.h
# End Source File
# Begin Source File

SOURCE=.\beamrender_plain.cpp
# End Source File
# Begin Source File

SOURCE=.\beamrender_plain.h
# End Source File
# Begin Source File

SOURCE=.\beamrender_runner.cpp
# End Source File
# Begin Source File

SOURCE=.\beamrender_runner.h
# End Source File
# Begin Source File

SOURCE=.\beamrender_shroud.cpp
# End Source File
# Begin Source File

SOURCE=.\beamrender_shroud.h
# End Source File
# Begin Source File

SOURCE=.\beamrender_weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\beamrender_weapon.h
# End Source File
# Begin Source File

SOURCE=.\bloodsimulate.cpp
# End Source File
# Begin Source File

SOURCE=.\bloodsimulate.h
# End Source File
# Begin Source File

SOURCE=.\chunksimulate.cpp
# End Source File
# Begin Source File

SOURCE=.\chunksimulate.h
# End Source File
# Begin Source File

SOURCE=.\dustsimulate.cpp
# End Source File
# Begin Source File

SOURCE=.\dustsimulate.h
# End Source File
# Begin Source File

SOURCE=.\embersimulate.cpp
# End Source File
# Begin Source File

SOURCE=.\embersimulate.h
# End Source File
# Begin Source File

SOURCE=.\geometryrender.cpp
# End Source File
# Begin Source File

SOURCE=.\geometryrender.h
# End Source File
# Begin Source File

SOURCE=.\groundspriterender.cpp
# End Source File
# Begin Source File

SOURCE=.\groundspriterender.h
# End Source File
# Begin Source File

SOURCE=.\particle.cpp
# End Source File
# Begin Source File

SOURCE=.\particle.h
# End Source File
# Begin Source File

SOURCE=.\particlerender.cpp
# End Source File
# Begin Source File

SOURCE=.\particlerender.h
# End Source File
# Begin Source File

SOURCE=.\particlesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\particlesystem.h
# End Source File
# Begin Source File

SOURCE=.\particlesystem_priv.h
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_coregame.h"
# End Source File
# Begin Source File

SOURCE=.\smokesimulate.cpp
# End Source File
# Begin Source File

SOURCE=.\smokesimulate.h
# End Source File
# Begin Source File

SOURCE=.\spriterender.cpp
# End Source File
# Begin Source File

SOURCE=.\spriterender.h
# End Source File
# Begin Source File

SOURCE=.\trailrender.cpp
# End Source File
# Begin Source File

SOURCE=.\trailrender.h
# End Source File
# Begin Source File

SOURCE=.\waterspriterender.cpp
# End Source File
# Begin Source File

SOURCE=.\waterspriterender.h
# End Source File
# End Target
# End Project
