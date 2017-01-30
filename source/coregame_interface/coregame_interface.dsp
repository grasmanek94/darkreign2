# Microsoft Developer Studio Project File - Name="coregame_interface" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_interface - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_interface.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_interface.mak" CFG="coregame_interface - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_interface - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_interface - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_interface - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_interface", VKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_interface - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_interface"
# PROP Intermediate_Dir "../Release/coregame_interface"
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

!ELSEIF  "$(CFG)" == "coregame_interface - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_interface"
# PROP Intermediate_Dir "../Debug/coregame_interface"
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

!ELSEIF  "$(CFG)" == "coregame_interface - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_interface___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_interface___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_interface"
# PROP Intermediate_Dir "../Development/coregame_interface"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Fa"../Release/coregame_objects/" /Fp"../Release/coregame_objects/coregame_objects.pch" /Yu"stdinc_coregame.h" /Fo"../Release/coregame_objects/" /Fd"../Release/coregame_objects/" /FD /c
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

# Name "coregame_interface - Win32 Release"
# Name "coregame_interface - Win32 Debug"
# Name "coregame_interface - Win32 Development"
# Begin Group "client"

# PROP Default_Filter ""
# Begin Group "client controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\client_clustermap.cpp
# End Source File
# Begin Source File

SOURCE=.\client_clustermap.h
# End Source File
# Begin Source File

SOURCE=.\client_commandpoints.cpp
# End Source File
# Begin Source File

SOURCE=.\client_commandpoints.h
# End Source File
# Begin Source File

SOURCE=.\client_construction.cpp
# End Source File
# Begin Source File

SOURCE=.\client_construction.h
# End Source File
# Begin Source File

SOURCE=.\client_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\client_debriefing.cpp
# End Source File
# Begin Source File

SOURCE=.\client_debriefing.h
# End Source File
# Begin Source File

SOURCE=.\client_displayobjectives.cpp
# End Source File
# Begin Source File

SOURCE=.\client_displayobjectives.h
# End Source File
# Begin Source File

SOURCE=.\client_facility.cpp
# End Source File
# Begin Source File

SOURCE=.\client_facility.h
# End Source File
# Begin Source File

SOURCE=.\client_orderlag.cpp
# End Source File
# Begin Source File

SOURCE=.\client_orderlag.h
# End Source File
# Begin Source File

SOURCE=.\client_power.cpp
# End Source File
# Begin Source File

SOURCE=.\client_power.h
# End Source File
# Begin Source File

SOURCE=.\client_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\client_resource.h
# End Source File
# Begin Source File

SOURCE=.\client_scorchcontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\client_scorchcontrol.h
# End Source File
# Begin Source File

SOURCE=.\client_squadcontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\client_squadcontrol.h
# End Source File
# Begin Source File

SOURCE=.\client_tactical.cpp
# End Source File
# Begin Source File

SOURCE=.\client_tactical.h
# End Source File
# Begin Source File

SOURCE=.\client_timeofday.cpp
# End Source File
# Begin Source File

SOURCE=.\client_timeofday.h
# End Source File
# Begin Source File

SOURCE=.\client_unitcontext.cpp
# End Source File
# Begin Source File

SOURCE=.\client_unitcontext.h
# End Source File
# Begin Source File

SOURCE=.\client_unitselection.cpp
# End Source File
# Begin Source File

SOURCE=.\client_unitselection.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\client_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\client_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\client_display.cpp
# End Source File
# Begin Source File

SOURCE=.\client_events.cpp
# End Source File
# Begin Source File

SOURCE=.\client_group.cpp
# End Source File
# Begin Source File

SOURCE=.\client_hud.cpp
# End Source File
# Begin Source File

SOURCE=.\client_private.h
# End Source File
# Begin Source File

SOURCE=.\client_trail.cpp
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Group "common controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common_calendar.cpp
# End Source File
# Begin Source File

SOURCE=.\common_calendar.h
# End Source File
# Begin Source File

SOURCE=.\common_cursorpos.cpp
# End Source File
# Begin Source File

SOURCE=.\common_cursorpos.h
# End Source File
# Begin Source File

SOURCE=.\common_gamewindow.cpp
# End Source File
# Begin Source File

SOURCE=.\common_gamewindow.h
# End Source File
# Begin Source File

SOURCE=.\common_mapwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\common_mapwindow.h
# End Source File
# Begin Source File

SOURCE=.\common_messagewindow.cpp
# End Source File
# Begin Source File

SOURCE=.\common_messagewindow.h
# End Source File
# Begin Source File

SOURCE=.\common_prereqlist.cpp
# End Source File
# Begin Source File

SOURCE=.\common_prereqlist.h
# End Source File
# Begin Source File

SOURCE=.\common_prereqtree.cpp
# End Source File
# Begin Source File

SOURCE=.\common_prereqtree.h
# End Source File
# Begin Source File

SOURCE=.\common_stats.cpp
# End Source File
# Begin Source File

SOURCE=.\common_stats.h
# End Source File
# Begin Source File

SOURCE=.\common_teamlistbox.cpp
# End Source File
# Begin Source File

SOURCE=.\common_teamlistbox.h
# End Source File
# Begin Source File

SOURCE=.\common_typelist.cpp
# End Source File
# Begin Source File

SOURCE=.\common_typelist.h
# End Source File
# Begin Source File

SOURCE=.\common_typepreview.cpp
# End Source File
# Begin Source File

SOURCE=.\common_typepreview.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\common.cpp
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\common_cycle.cpp
# End Source File
# Begin Source File

SOURCE=.\common_display.cpp
# End Source File
# End Group
# Begin Group "studio"

# PROP Default_Filter ""
# Begin Group "studio brushes"

# PROP Default_Filter ""
# Begin Group "base brushes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\studio_brush.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_apply.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_apply.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_applycell.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_applycell.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_area.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_area.h
# End Source File
# End Group
# Begin Group "user brushes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\studio_brush_curve.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_curve.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_objects.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_objects.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_overlay.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_overlay.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_pathsearch.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_pathsearch.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_region.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_region.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_tag.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_tag.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_terrain.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_terraintweak.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_terraintweak.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_trail.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_trail.h
# End Source File
# Begin Source File

SOURCE=.\studio_brush_water.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_brush_water.h
# End Source File
# End Group
# Begin Group "history"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\studio_history.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_history.h
# End Source File
# Begin Source File

SOURCE=.\studio_history_objects.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_history_objects.h
# End Source File
# Begin Source File

SOURCE=.\studio_history_terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_history_terrain.h
# End Source File
# End Group
# End Group
# Begin Group "studio controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\studio_attachlist.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_attachlist.h
# End Source File
# Begin Source File

SOURCE=.\studio_bookmarklist.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_bookmarklist.h
# End Source File
# Begin Source File

SOURCE=.\studio_bookmarklistbox.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_bookmarklistbox.h
# End Source File
# Begin Source File

SOURCE=.\studio_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_controls.h
# End Source File
# Begin Source File

SOURCE=.\studio_footeditor.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_footeditor.h
# End Source File
# Begin Source File

SOURCE=.\studio_lighteditor.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_lighteditor.h
# End Source File
# Begin Source File

SOURCE=.\studio_mission.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_mission.h
# End Source File
# Begin Source File

SOURCE=.\studio_objecttweak.h
# End Source File
# Begin Source File

SOURCE=.\studio_objecttweak_baseobj.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_objecttweak_mapobj.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_objecttweak_resourceobj.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_objecttweak_unitobj.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_regionlist.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_regionlist.h
# End Source File
# Begin Source File

SOURCE=.\studio_taglist.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_taglist.h
# End Source File
# Begin Source File

SOURCE=.\studio_teameditor.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_teameditor.h
# End Source File
# Begin Source File

SOURCE=.\studio_teamlist.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_teamlist.h
# End Source File
# Begin Source File

SOURCE=.\studio_weather.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_weather.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\studio.cpp
# End Source File
# Begin Source File

SOURCE=.\studio.h
# End Source File
# Begin Source File

SOURCE=.\studio_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_display.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_event.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_private.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_coregame.h"
# End Source File
# Begin Source File

SOURCE=..\coregame\stdinc_coregame.h
# End Source File
# End Target
# End Project
