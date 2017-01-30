# Microsoft Developer Studio Project File - Name="coregame_ai" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_ai - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_ai.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_ai.mak" CFG="coregame_ai - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_ai - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_ai - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_ai - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_ai", UKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_ai - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_ai"
# PROP Intermediate_Dir "../Release/coregame_ai"
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

!ELSEIF  "$(CFG)" == "coregame_ai - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_ai"
# PROP Intermediate_Dir "../Debug/coregame_ai"
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

!ELSEIF  "$(CFG)" == "coregame_ai - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_ai___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_ai___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_ai"
# PROP Intermediate_Dir "../Development/coregame_ai"
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

# Name "coregame_ai - Win32 Release"
# Name "coregame_ai - Win32 Debug"
# Name "coregame_ai - Win32 Development"
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_debug.h
# End Source File
# Begin Source File

SOURCE=.\ai_debug_info.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_debug_info.h
# End Source File
# Begin Source File

SOURCE=.\ai_debug_teamlist.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_debug_teamlist.h
# End Source File
# End Group
# Begin Group "formation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\formation.cpp
# End Source File
# Begin Source File

SOURCE=.\formation.h
# End Source File
# End Group
# Begin Group "strategic"

# PROP Default_Filter ""
# Begin Group "strategic asset"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_asset.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset.h
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_manager.h
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request.h
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_force.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_forcemap.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_squad.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_tag.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_type.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_asset_request_typebase.cpp
# End Source File
# End Group
# Begin Group "strategic base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_base.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_manager.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer_manifest.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer_manifests.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer_type.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_orderer_type.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_recycler.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_recycler.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_state.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_state.h
# End Source File
# Begin Source File

SOURCE=.\strategic_base_token.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_base_token.h
# End Source File
# End Group
# Begin Group "strategic bombardier"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_bombardier.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier.h
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_manager.h
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_offmap.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_ruleset.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_ruleset.h
# End Source File
# Begin Source File

SOURCE=.\strategic_bombardier_unit.cpp
# End Source File
# End Group
# Begin Group "strategic intel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_intel.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_intel.h
# End Source File
# Begin Source File

SOURCE=.\strategic_intel_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_intel_manager.h
# End Source File
# End Group
# Begin Group "strategic placement"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_placement.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_cluster.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_cluster.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_clusterset.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_clusterset.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_locator.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_locator.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_manager.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_manager_clusterinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_manager_clusterinfo.h
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_ruleset.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_placement_ruleset.h
# End Source File
# End Group
# Begin Group "strategic resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_resource.h
# End Source File
# Begin Source File

SOURCE=.\strategic_resource_decomposition.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_resource_decomposition.h
# End Source File
# Begin Source File

SOURCE=.\strategic_resource_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_resource_manager.h
# End Source File
# End Group
# Begin Group "strategic rule"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_rule.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_rule.h
# End Source File
# Begin Source File

SOURCE=.\strategic_rule_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_rule_manager.h
# End Source File
# End Group
# Begin Group "strategic script"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_script.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script.h
# End Source File
# Begin Source File

SOURCE=.\strategic_script_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_manager.h
# End Source File
# Begin Source File

SOURCE=.\strategic_script_recruiter.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_recruiter.h
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state.h
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state_actions.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state_conditions.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state_settings.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_script_state_transitions.cpp
# End Source File
# End Group
# Begin Group "strategic transport"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_transport.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_transport.h
# End Source File
# Begin Source File

SOURCE=.\strategic_transport_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_transport_manager.h
# End Source File
# End Group
# Begin Group "strategic water"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\strategic_water.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_water.h
# End Source File
# Begin Source File

SOURCE=.\strategic_water_decomposition.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_water_decomposition.h
# End Source File
# Begin Source File

SOURCE=.\strategic_water_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_water_manager.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\strategic.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic.h
# End Source File
# Begin Source File

SOURCE=.\strategic_config.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_config.h
# End Source File
# Begin Source File

SOURCE=.\strategic_location.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_location.h
# End Source File
# Begin Source File

SOURCE=.\strategic_notification.h
# End Source File
# Begin Source File

SOURCE=.\strategic_object.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_object.h
# End Source File
# Begin Source File

SOURCE=.\strategic_pqueue.h
# End Source File
# Begin Source File

SOURCE=.\strategic_private.h
# End Source File
# Begin Source File

SOURCE=.\strategic_util.cpp
# End Source File
# Begin Source File

SOURCE=.\strategic_util.h
# End Source File
# Begin Source File

SOURCE=.\strategic_weighting.h
# End Source File
# End Group
# Begin Group "tactical"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tactical.cpp
# End Source File
# Begin Source File

SOURCE=.\tactical.h
# End Source File
# Begin Source File

SOURCE=.\tactical_private.h
# End Source File
# Begin Source File

SOURCE=.\tactical_process.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\ai.h
# End Source File
# Begin Source File

SOURCE=.\ai_map.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_map.h
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"stdinc_coregame.h"
# End Source File
# Begin Source File

SOURCE=..\coregame\stdinc_coregame.h
# End Source File
# End Target
# End Project
