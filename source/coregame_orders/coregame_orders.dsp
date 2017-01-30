# Microsoft Developer Studio Project File - Name="coregame_orders" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=coregame_orders - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coregame_orders.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coregame_orders.mak" CFG="coregame_orders - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coregame_orders - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_orders - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "coregame_orders - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/coregame_orders", XKCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coregame_orders - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/coregame_orders"
# PROP Intermediate_Dir "../Release/coregame_orders"
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

!ELSEIF  "$(CFG)" == "coregame_orders - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/coregame_objects"
# PROP Intermediate_Dir "../Debug/coregame_objects"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"stdinc_coregame.h" /D "WIN32" /D "DEVELOPMENT" /Fp"../Debug/coregame_orders/coregame_orders.pch" /Yu"stdinc_coregame.h" /Fo"../Debug/coregame_orders/" /Fd"../Debug/coregame_orders/" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Debug\coregame_orders\coregame_orders.lib"

!ELSEIF  "$(CFG)" == "coregame_orders - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "coregame_orders___Win32_Development"
# PROP BASE Intermediate_Dir "coregame_orders___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/coregame_orders"
# PROP Intermediate_Dir "../Development/coregame_orders"
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

# Name "coregame_orders - Win32 Release"
# Name "coregame_orders - Win32 Debug"
# Name "coregame_orders - Win32 Development"
# Begin Group "game orders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\orders_game.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game.h
# End Source File
# Begin Source File

SOURCE=.\orders_game_addselected.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_attack.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_board.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_build.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_clearselected.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_collect.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_constructor.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_delete.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_dumpsync.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_ejectspy.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_explore.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_fire.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_formation.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_give.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_guard.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_infiltrate.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_morph.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_move.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_offmap.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_pause.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_playerleft.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_postevent.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_powerdown.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_recycle.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_restore.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_restoremobile.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_restorestatic.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_return.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_revealspy.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_scatter.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_selfdestruct.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_setrally.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_setrestore.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_steponce.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_stop.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_store.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_tactical.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_trail.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_trailpoints.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_transport.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_turn.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_unload.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_upgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_game_wall.cpp
# End Source File
# End Group
# Begin Group "squad orders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\orders_squad.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad.h
# End Source File
# Begin Source File

SOURCE=.\orders_squad_addselected.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_attack.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_board.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_create.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_defect.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_delete.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_destroy.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_empty.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_explore.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_followtag.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_formation.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_guard.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_move.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_notify.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_removeselected.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_restore.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_spawn.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_stop.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_tactical.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_trail.cpp
# End Source File
# Begin Source File

SOURCE=.\orders_squad_transportspawn.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\orders.cpp
# End Source File
# Begin Source File

SOURCE=.\orders.h
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "coregame_orders - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "coregame_orders - Win32 Debug"

# ADD CPP /Yc"stdinc_coregame.h"

!ELSEIF  "$(CFG)" == "coregame_orders - Win32 Development"

# ADD CPP /Yc"stdinc_coregame.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\coregame\stdinc_coregame.h
# End Source File
# End Target
# End Project
