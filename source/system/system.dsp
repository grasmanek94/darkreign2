# Microsoft Developer Studio Project File - Name="system" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=system - Win32 Development
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "system.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "system.mak" CFG="system - Win32 Development"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "system - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "system - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "system - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/system", GAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "system - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/system"
# PROP Intermediate_Dir "../Release/system"
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

!ELSEIF  "$(CFG)" == "system - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/system"
# PROP Intermediate_Dir "../Debug/system"
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

!ELSEIF  "$(CFG)" == "system - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "system___Win32_Development"
# PROP BASE Intermediate_Dir "system___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/system"
# PROP Intermediate_Dir "../Development/system"
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

# Name "system - Win32 Release"
# Name "system - Win32 Debug"
# Name "system - Win32 Development"
# Begin Group "containers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\array.h
# End Source File
# Begin Source File

SOURCE=.\heap.cpp
# End Source File
# Begin Source File

SOURCE=.\heap.h
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\nlist.h
# End Source File
# Begin Source File

SOURCE=.\ntree.h
# End Source File
# Begin Source File

SOURCE=.\queue.h
# End Source File
# Begin Source File

SOURCE=.\reaperlist.h
# End Source File
# Begin Source File

SOURCE=.\stack.h
# End Source File
# Begin Source File

SOURCE=.\tree.h
# End Source File
# Begin Source File

SOURCE=.\weightedlist.h
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\blockfile.cpp
# End Source File
# Begin Source File

SOURCE=.\blockfile.h
# End Source File
# Begin Source File

SOURCE=.\configfile.cpp
# End Source File
# Begin Source File

SOURCE=.\configfile.h
# End Source File
# Begin Source File

SOURCE=.\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\dlgtemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgtemplate.h
# End Source File
# Begin Source File

SOURCE=.\dtrack.cpp
# End Source File
# Begin Source File

SOURCE=.\dtrack.h
# End Source File
# Begin Source File

SOURCE=.\fixme.cpp
# End Source File
# Begin Source File

SOURCE=.\fixme.h
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\random.cpp
# End Source File
# Begin Source File

SOURCE=.\random.h
# End Source File
# Begin Source File

SOURCE=.\str.cpp
# End Source File
# Begin Source File

SOURCE=.\str.h
# End Source File
# Begin Source File

SOURCE=.\strbuf.h
# End Source File
# Begin Source File

SOURCE=.\strcrc.h
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\version.cpp
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "filesys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\filesrcdir.cpp
# End Source File
# Begin Source File

SOURCE=.\filesrcdir.h
# End Source File
# Begin Source File

SOURCE=.\filesrcpack.cpp
# End Source File
# Begin Source File

SOURCE=.\filesrcpack.h
# End Source File
# Begin Source File

SOURCE=.\filesrcstream.cpp
# End Source File
# Begin Source File

SOURCE=.\filesrcstream.h
# End Source File
# Begin Source File

SOURCE=.\filesys.cpp
# End Source File
# Begin Source File

SOURCE=.\filesys.h
# End Source File
# Begin Source File

SOURCE=.\filesys_private.h
# End Source File
# Begin Source File

SOURCE=.\filetypes.h
# End Source File
# Begin Source File

SOURCE=.\packdef.h
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\clock.cpp
# End Source File
# Begin Source File

SOURCE=.\clock.h
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\debug_memory.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_memory.h
# End Source File
# Begin Source File

SOURCE=.\debug_memory_win32.h
# End Source File
# Begin Source File

SOURCE=.\debug_symbol.cpp
# End Source File
# Begin Source File

SOURCE=.\debug_symbol.h
# End Source File
# Begin Source File

SOURCE=.\debug_win32.h
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\filemap.cpp
# End Source File
# Begin Source File

SOURCE=.\filemap.h
# End Source File
# Begin Source File

SOURCE=.\hardware.cpp
# End Source File
# Begin Source File

SOURCE=.\hardware.h
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\mono.cpp
# End Source File
# Begin Source File

SOURCE=.\mono.h
# End Source File
# Begin Source File

SOURCE=.\profile.cpp
# End Source File
# Begin Source File

SOURCE=.\profile.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\system.h
# End Source File
# Begin Source File

SOURCE=.\win32.h
# End Source File
# End Group
# Begin Group "parse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fscope.cpp
# End Source File
# Begin Source File

SOURCE=.\fscope.h
# End Source File
# Begin Source File

SOURCE=.\ptree.cpp
# End Source File
# Begin Source File

SOURCE=.\ptree.h
# End Source File
# Begin Source File

SOURCE=.\stdparse.cpp
# End Source File
# Begin Source File

SOURCE=.\stdparse.h
# End Source File
# Begin Source File

SOURCE=.\tbuf.cpp
# End Source File
# Begin Source File

SOURCE=.\tbuf.h
# End Source File
# Begin Source File

SOURCE=.\vnode.cpp
# End Source File
# Begin Source File

SOURCE=.\vnode.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\pchbuilder.cpp

!IF  "$(CFG)" == "system - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "system - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "system - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\std.h
# End Source File
# End Target
# End Project
