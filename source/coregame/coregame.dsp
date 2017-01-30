# Microsoft Developer Studio Project File - Name="coregame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame.mak" CFG="coregame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame", UOAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame"
# PROP Intermediate_Dir "../Release/coregame"
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

!ELSEIF  "$(CFG)" == "coregame - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame"
# PROP Intermediate_Dir "../Debug/coregame"
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

!ELSEIF  "$(CFG)" == "coregame - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame___Win32_Development"
# PROP BASE Intermediate_Dir "coregame___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame"
# PROP Intermediate_Dir "../Development/coregame"
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

# Name "coregame - Win32 Release"
# Name "coregame - Win32 Debug"
# Name "coregame - Win32 Development"
# Begin Group "connected region"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\blobs.cpp
# End Source File
# Begin Source File

SOURCE=.\blobs.h
# End Source File
# Begin Source File

SOURCE=.\connectedregion.cpp
# End Source File
# Begin Source File

SOURCE=.\connectedregion.h
# End Source File
# End Group
# Begin Group "environment"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\environment.cpp
# End Source File
# Begin Source File

SOURCE=.\environment.h
# End Source File
# Begin Source File

SOURCE=.\environment_light.cpp
# End Source File
# Begin Source File

SOURCE=.\environment_light.h
# End Source File
# Begin Source File

SOURCE=.\environment_quake.cpp
# End Source File
# Begin Source File

SOURCE=.\environment_quake.h
# End Source File
# Begin Source File

SOURCE=.\environment_rain.cpp
# End Source File
# Begin Source File

SOURCE=.\environment_rain.h
# End Source File
# Begin Source File

SOURCE=.\environment_rain_priv.h
# End Source File
# Begin Source File

SOURCE=.\environment_time.cpp
# End Source File
# Begin Source File

SOURCE=.\environment_time.h
# End Source File
# End Group
# Begin Group "footprint"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\footprint.cpp
# End Source File
# Begin Source File

SOURCE=.\footprint.h
# End Source File
# Begin Source File

SOURCE=.\footprint_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\footprint_layer.cpp
# End Source File
# Begin Source File

SOURCE=.\footprint_placement.cpp
# End Source File
# Begin Source File

SOURCE=.\footprint_private.h
# End Source File
# Begin Source File

SOURCE=.\footprint_type.cpp
# End Source File
# End Group
# Begin Group "fx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fx.cpp
# End Source File
# Begin Source File

SOURCE=.\fx.h
# End Source File
# Begin Source File

SOURCE=.\fx_object.h
# End Source File
# Begin Source File

SOURCE=.\fx_type.h
# End Source File
# Begin Source File

SOURCE=.\meshfx.cpp
# End Source File
# Begin Source File

SOURCE=.\meshfx_object.h
# End Source File
# Begin Source File

SOURCE=.\meshfx_type.h
# End Source File
# Begin Source File

SOURCE=.\particlefx.cpp
# End Source File
# Begin Source File

SOURCE=.\particlefx_object.h
# End Source File
# Begin Source File

SOURCE=.\particlefx_type.h
# End Source File
# Begin Source File

SOURCE=.\soundfx.cpp
# End Source File
# Begin Source File

SOURCE=.\soundfx_object.h
# End Source File
# Begin Source File

SOURCE=.\soundfx_type.h
# End Source File
# End Group
# Begin Group "game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\game.cpp
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\game_config.cpp
# End Source File
# Begin Source File

SOURCE=.\game_config.h
# End Source File
# Begin Source File

SOURCE=.\game_preview.cpp
# End Source File
# Begin Source File

SOURCE=.\game_preview.h
# End Source File
# Begin Source File

SOURCE=.\game_rc.cpp
# End Source File
# Begin Source File

SOURCE=.\game_rc.h
# End Source File
# Begin Source File

SOURCE=.\gameconstants.h
# End Source File
# Begin Source File

SOURCE=.\missionvar.cpp
# End Source File
# Begin Source File

SOURCE=.\missionvar.h
# End Source File
# End Group
# Begin Group "message"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\message.cpp
# End Source File
# Begin Source File

SOURCE=.\message.h
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\propertylist.h
# End Source File
# Begin Source File

SOURCE=.\ray.cpp
# End Source File
# Begin Source File

SOURCE=.\ray.h
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# End Group
# Begin Group "movement"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\movement.cpp
# End Source File
# Begin Source File

SOURCE=.\movement.h
# End Source File
# Begin Source File

SOURCE=.\movement_pathfollow.cpp
# End Source File
# Begin Source File

SOURCE=.\movement_pathfollow.h
# End Source File
# End Group
# Begin Group "pathsearch"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pathsearch.cpp
# End Source File
# Begin Source File

SOURCE=.\pathsearch.h
# End Source File
# Begin Source File

SOURCE=.\pathsearch_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\pathsearch_finder.cpp
# End Source File
# Begin Source File

SOURCE=.\pathsearch_path.cpp
# End Source File
# Begin Source File

SOURCE=.\pathsearch_pqueue.h
# End Source File
# Begin Source File

SOURCE=.\pathsearch_priv.h
# End Source File
# End Group
# Begin Group "physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\collisionctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\collisionctrl.h
# End Source File
# Begin Source File

SOURCE=.\physics_const.h
# End Source File
# Begin Source File

SOURCE=.\physics_type.cpp
# End Source File
# Begin Source File

SOURCE=.\physics_type.h
# End Source File
# Begin Source File

SOURCE=.\physicsctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\physicsctrl.h
# End Source File
# End Group
# Begin Group "sight"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sight.cpp
# End Source File
# Begin Source File

SOURCE=.\sight.h
# End Source File
# Begin Source File

SOURCE=.\sight_bytemap.cpp
# End Source File
# Begin Source File

SOURCE=.\sight_bytemap.h
# End Source File
# Begin Source File

SOURCE=.\sight_map.cpp
# End Source File
# Begin Source File

SOURCE=.\sight_map.h
# End Source File
# Begin Source File

SOURCE=.\sight_tables.cpp
# End Source File
# Begin Source File

SOURCE=.\sight_tables.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\claim.cpp
# End Source File
# Begin Source File

SOURCE=.\claim.h
# End Source File
# Begin Source File

SOURCE=.\coregame.cpp
# End Source File
# Begin Source File

SOURCE=.\coregame.h
# End Source File
# Begin Source File

SOURCE=.\gamebabel.cpp
# End Source File
# Begin Source File

SOURCE=.\gamebabel.h
# End Source File
# Begin Source File

SOURCE=.\instance_modifier.cpp
# End Source File
# Begin Source File

SOURCE=.\instance_modifier.h
# End Source File
# Begin Source File

SOURCE=.\instance_modifier_private.h
# End Source File
# Begin Source File

SOURCE=.\movetable.cpp
# End Source File
# Begin Source File

SOURCE=.\movetable.h
# End Source File
# Begin Source File

SOURCE=.\promote.h
# End Source File
# Begin Source File

SOURCE=.\resolver.h
# End Source File
# Begin Source File

SOURCE=.\worldctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\worldctrl.h
# End Source File
# Begin Source File

SOURCE=.\worldctrldec.h
# End Source File
# Begin Source File

SOURCE=.\worldload.cpp
# End Source File
# Begin Source File

SOURCE=.\worldload.h
# End Source File
# End Group
# Begin Group "team"

# PROP Default_Filter ""
# Begin Group "conditions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\condition.cpp
# End Source File
# Begin Source File

SOURCE=.\condition.h
# End Source File
# Begin Source File

SOURCE=.\condition_cineractive.cpp
# End Source File
# Begin Source File

SOURCE=.\condition_game.cpp
# End Source File
# Begin Source File

SOURCE=.\condition_private.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\action.cpp
# End Source File
# Begin Source File

SOURCE=.\action.h
# End Source File
# Begin Source File

SOURCE=.\displayobjective.cpp
# End Source File
# Begin Source File

SOURCE=.\displayobjective.h
# End Source File
# Begin Source File

SOURCE=.\objective.cpp
# End Source File
# Begin Source File

SOURCE=.\objective.h
# End Source File
# Begin Source File

SOURCE=.\player.cpp
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\power.cpp
# End Source File
# Begin Source File

SOURCE=.\power.h
# End Source File
# Begin Source File

SOURCE=.\prereq.cpp
# End Source File
# Begin Source File

SOURCE=.\prereq.h
# End Source File
# Begin Source File

SOURCE=.\radio.cpp
# End Source File
# Begin Source File

SOURCE=.\radio.h
# End Source File
# Begin Source File

SOURCE=.\relation.cpp
# End Source File
# Begin Source File

SOURCE=.\relation.h
# End Source File
# Begin Source File

SOURCE=.\sideplacement.cpp
# End Source File
# Begin Source File

SOURCE=.\sideplacement.h
# End Source File
# Begin Source File

SOURCE=.\team.cpp
# End Source File
# Begin Source File

SOURCE=.\team.h
# End Source File
# Begin Source File

SOURCE=.\unitlimits.cpp
# End Source File
# Begin Source File

SOURCE=.\unitlimits.h
# End Source File
# End Group
# Begin Group "terrain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\terraindata.cpp
# End Source File
# Begin Source File

SOURCE=.\terraindata.h
# End Source File
# Begin Source File

SOURCE=.\terraindatadec.h
# End Source File
# Begin Source File

SOURCE=.\terraingroup.cpp
# End Source File
# Begin Source File

SOURCE=.\terraingroup.h
# End Source File
# End Group
# Begin Group "viewer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\viewer.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer.h
# End Source File
# Begin Source File

SOURCE=.\viewer_action.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_base.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_cinema.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_custom.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_padlock.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_playback.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_private.h
# End Source File
# Begin Source File

SOURCE=.\viewer_standard.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer_track.cpp
# End Source File
# End Group
# Begin Group "weapons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\armourclass.cpp
# End Source File
# Begin Source File

SOURCE=.\armourclass.h
# End Source File
# Begin Source File

SOURCE=.\damage.cpp
# End Source File
# Begin Source File

SOURCE=.\damage.h
# End Source File
# Begin Source File

SOURCE=.\firing.cpp
# End Source File
# Begin Source File

SOURCE=.\firing.h
# End Source File
# Begin Source File

SOURCE=.\target.cpp
# End Source File
# Begin Source File

SOURCE=.\target.h
# End Source File
# Begin Source File

SOURCE=.\weapon.h
# End Source File
# Begin Source File

SOURCE=.\weapon_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon_object.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon_type.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_coregame.h"
# End Source File
# Begin Source File

SOURCE=.\stdinc_coregame.h
# End Source File
# End Target
# End Project
