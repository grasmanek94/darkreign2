# Microsoft Developer Studio Project File - Name="sound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sound - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak" CFG="sound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sound - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/sound", REAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sound - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/sound"
# PROP Intermediate_Dir "../Release/sound"
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

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/sound"
# PROP Intermediate_Dir "../Debug/sound"
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

!ELSEIF  "$(CFG)" == "sound - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sound___Win32_Development"
# PROP BASE Intermediate_Dir "sound___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/sound"
# PROP Intermediate_Dir "../Development/sound"
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

# Name "sound - Win32 Release"
# Name "sound - Win32 Debug"
# Name "sound - Win32 Development"
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "sound - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "sound - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\sound_digital.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_effect.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_output.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_record.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_reserved.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_digital_stream.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_private.h
# End Source File
# Begin Source File

SOURCE=.\sound_redbook.cpp
# End Source File
# End Target
# End Project
