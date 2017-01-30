# Microsoft Developer Studio Project File - Name="multiplayer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=multiplayer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "multiplayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "multiplayer.mak" CFG="multiplayer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "multiplayer - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "multiplayer - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "multiplayer - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/multiplayer", CFAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "multiplayer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/multiplayer"
# PROP Intermediate_Dir "../Release/multiplayer"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../styxnet" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "multiplayer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/multiplayer"
# PROP Intermediate_Dir "../Debug/multiplayer"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../styxnet" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "multiplayer - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "multiplayer___Win32_Development"
# PROP BASE Intermediate_Dir "multiplayer___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/multiplayer"
# PROP Intermediate_Dir "../Development/multiplayer"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../styxnet" /I "../util" /I "../won" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"std.h" /FD /c
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

# Name "multiplayer - Win32 Release"
# Name "multiplayer - Win32 Debug"
# Name "multiplayer - Win32 Development"
# Begin Group "multiplayer"

# PROP Default_Filter ""
# Begin Group "multiplayer cmd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_cmd.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_cmd_private.h
# End Source File
# End Group
# Begin Group "multiplayer controls"

# PROP Default_Filter ""
# Begin Group "setup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_controls_color.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_color.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_context.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_context.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_hostconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_hostconfig.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_mission.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_mission.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_playerlist.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_playerlist.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_propertylist.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_propertylist.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_transferlist.cpp
# End Source File
# End Group
# Begin Group "stats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_controls_localpings.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_localpings.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_playerinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_playerinfo.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_report.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_report.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_transferlist.h
# End Source File
# End Group
# Begin Group "sessions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_controls_addressbook.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_addressbook.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_sessions.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_sessions.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\multiplayer_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_chatedit.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_chatedit.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_download.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_download.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_earth.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_earth.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_options.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_options.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_synclist.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_controls_synclist.h
# End Source File
# End Group
# Begin Group "multiplayer data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_data.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_data.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_data_private.h
# End Source File
# End Group
# Begin Group "multiplayer network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_network.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_network.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_network_private.h
# End Source File
# End Group
# Begin Group "multiplayer transfer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\multiplayer_transfer.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_transfer.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_transfer_private.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\multiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_addressbook.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_addressbook.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_download.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_download.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_host.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_host.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_pingdisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_pingdisplay.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_private.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer_settings.cpp
# End Source File
# Begin Source File

SOURCE=.\multiplayer_settings.h
# End Source File
# End Group
# Begin Group "won"

# PROP Default_Filter ""
# Begin Group "won controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\won_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\won_controls.h
# End Source File
# Begin Source File

SOURCE=.\won_controls_gameinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\won_controls_gameinfo.h
# End Source File
# Begin Source File

SOURCE=.\won_controls_gamelist.cpp
# End Source File
# Begin Source File

SOURCE=.\won_controls_gamelist.h
# End Source File
# Begin Source File

SOURCE=.\won_controls_playerlist.cpp
# End Source File
# Begin Source File

SOURCE=.\won_controls_playerlist.h
# End Source File
# Begin Source File

SOURCE=.\won_controls_roomlist.cpp
# End Source File
# Begin Source File

SOURCE=.\won_controls_roomlist.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\won.cpp
# End Source File
# Begin Source File

SOURCE=.\won.h
# End Source File
# Begin Source File

SOURCE=.\won_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\won_cmd.h
# End Source File
# Begin Source File

SOURCE=.\won_private.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "multiplayer - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "multiplayer - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "multiplayer - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# End Target
# End Project
