# Microsoft Developer Studio Project File - Name="interface" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=interface - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "interface.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "interface.mak" CFG="interface - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "interface - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "interface - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "interface - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/interface", MGAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "interface - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/interface"
# PROP Intermediate_Dir "../Release/interface"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_interface.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"stdinc_interface.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "interface - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/interface"
# PROP Intermediate_Dir "../Debug/interface"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_interface.h" /D "WIN32" /D "DEVELOPMENT" /Yu"stdinc_interface.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "interface - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "interface___Win32_Development"
# PROP BASE Intermediate_Dir "interface___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/interface"
# PROP Intermediate_Dir "../Development/interface"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_interface.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"stdinc_interface.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_interface.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"stdinc_interface.h" /FD /c
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

# Name "interface - Win32 Release"
# Name "interface - Win32 Debug"
# Name "interface - Win32 Development"
# Begin Group "base controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\icbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\icbutton.h
# End Source File
# Begin Source File

SOURCE=.\icclass.h
# End Source File
# Begin Source File

SOURCE=.\icdroplist.cpp
# End Source File
# Begin Source File

SOURCE=.\icdroplist.h
# End Source File
# Begin Source File

SOURCE=.\icedit.cpp
# End Source File
# Begin Source File

SOURCE=.\icedit.h
# End Source File
# Begin Source File

SOURCE=.\icgauge.cpp
# End Source File
# Begin Source File

SOURCE=.\icgauge.h
# End Source File
# Begin Source File

SOURCE=.\icgrid.cpp
# End Source File
# Begin Source File

SOURCE=.\icgrid.h
# End Source File
# Begin Source File

SOURCE=.\icgridwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\icgridwindow.h
# End Source File
# Begin Source File

SOURCE=.\iclistbox.cpp
# End Source File
# Begin Source File

SOURCE=.\iclistbox.h
# End Source File
# Begin Source File

SOURCE=.\iclistslider.cpp
# End Source File
# Begin Source File

SOURCE=.\iclistslider.h
# End Source File
# Begin Source File

SOURCE=.\icmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\icmenu.h
# End Source File
# Begin Source File

SOURCE=.\icmesh.cpp
# End Source File
# Begin Source File

SOURCE=.\icmesh.h
# End Source File
# Begin Source File

SOURCE=.\icmonoview.cpp
# End Source File
# Begin Source File

SOURCE=.\icmonoview.h
# End Source File
# Begin Source File

SOURCE=.\icontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\icontrol.h
# End Source File
# Begin Source File

SOURCE=.\icroot.cpp
# End Source File
# Begin Source File

SOURCE=.\icroot.h
# End Source File
# Begin Source File

SOURCE=.\icslider.cpp
# End Source File
# Begin Source File

SOURCE=.\icslider.h
# End Source File
# Begin Source File

SOURCE=.\icstatic.cpp
# End Source File
# Begin Source File

SOURCE=.\icstatic.h
# End Source File
# Begin Source File

SOURCE=.\icsystembutton.cpp
# End Source File
# Begin Source File

SOURCE=.\icsystembutton.h
# End Source File
# Begin Source File

SOURCE=.\ictabgroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ictabgroup.h
# End Source File
# Begin Source File

SOURCE=.\icticker.cpp
# End Source File
# Begin Source File

SOURCE=.\icticker.h
# End Source File
# Begin Source File

SOURCE=.\ictimer.cpp
# End Source File
# Begin Source File

SOURCE=.\ictimer.h
# End Source File
# Begin Source File

SOURCE=.\ictipwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ictipwindow.h
# End Source File
# Begin Source File

SOURCE=.\icwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\icwindow.h
# End Source File
# End Group
# Begin Group "console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\console_edit.cpp
# End Source File
# Begin Source File

SOURCE=.\console_edit.h
# End Source File
# Begin Source File

SOURCE=.\console_viewer.cpp
# End Source File
# Begin Source File

SOURCE=.\console_viewer.h
# End Source File
# End Group
# Begin Group "game controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ictetris.cpp
# End Source File
# Begin Source File

SOURCE=.\ictetris.h
# End Source File
# End Group
# Begin Group "input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\cursor.h
# End Source File
# Begin Source File

SOURCE=.\input.cpp
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\keybind.cpp
# End Source File
# Begin Source File

SOURCE=.\keybind.h
# End Source File
# End Group
# Begin Group "messagebox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iface_messagebox.cpp
# End Source File
# Begin Source File

SOURCE=.\iface_messagebox.h
# End Source File
# Begin Source File

SOURCE=.\messagebox_event.cpp
# End Source File
# Begin Source File

SOURCE=.\messagebox_event.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\fontsys.cpp
# End Source File
# Begin Source File

SOURCE=.\fontsys.h
# End Source File
# Begin Source File

SOURCE=.\iface.cpp
# End Source File
# Begin Source File

SOURCE=.\iface.h
# End Source File
# Begin Source File

SOURCE=.\iface_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\iface_controls.h
# End Source File
# Begin Source File

SOURCE=.\iface_priv.h
# End Source File
# Begin Source File

SOURCE=.\iface_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\iface_sound.h
# End Source File
# Begin Source File

SOURCE=.\iface_types.h
# End Source File
# Begin Source File

SOURCE=.\iface_util.cpp
# End Source File
# Begin Source File

SOURCE=.\iface_util.h
# End Source File
# Begin Source File

SOURCE=.\iftypes.h
# End Source File
# Begin Source File

SOURCE=.\ifvar.cpp
# End Source File
# Begin Source File

SOURCE=.\ifvar.h
# End Source File
# End Group
# Begin Group "system controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\colorbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\colorbutton.h
# End Source File
# Begin Source File

SOURCE=.\coloreditor.cpp
# End Source File
# Begin Source File

SOURCE=.\coloreditor.h
# End Source File
# Begin Source File

SOURCE=.\ickeyview.cpp
# End Source File
# Begin Source File

SOURCE=.\ickeyview.h
# End Source File
# Begin Source File

SOURCE=.\iconwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\iconwindow.h
# End Source File
# Begin Source File

SOURCE=.\icsoundconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\icsoundconfig.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_interface.h"
# End Source File
# Begin Source File

SOURCE=.\stdinc_interface.h
# End Source File
# End Target
# End Project
