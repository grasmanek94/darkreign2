# Microsoft Developer Studio Project File - Name="appmesh" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=appmesh - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "appmesh.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "appmesh.mak" CFG="appmesh - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "appmesh - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "appmesh - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "appmesh - Win32 Development" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/appmesh", RGBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "appmesh - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/appmesh"
# PROP Intermediate_Dir "../Release/appmesh"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "NDEBUG" /D "WIN32" /D "DOMRMGEN" /FAcs /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"../Release/appmesh/mesh.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -version=1,1 -comments="RELEASE"	rc version.rc
PostBuild_Cmds=..\tools\postbuild release\mesh Pandemic Mesh Viewer [Release]
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appmesh - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/appmesh"
# PROP Intermediate_Dir "../Debug/appmesh"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "DEVELOPMENT" /D "WIN32" /D "DOMRMGEN" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /out:"../Debug/appmesh/mesh.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -version=1,1 -comments="DEBUG"	rc version.rc
PostBuild_Cmds=..\tools\postbuild debug\mesh Pandemic Mesh Viewer [Debug]
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appmesh - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "appmesh___Win32_Development"
# PROP BASE Intermediate_Dir "appmesh___Win32_Development"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/appmesh"
# PROP Intermediate_Dir "../Development/appmesh"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /I "../meshview" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "NDEBUG" /D "DEVELOPMENT" /D "WIN32" /D "DOMRMGEN" /FAcs /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /pdb:none /map /machine:I386 /out:"Release/mesh.exe"
# ADD LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"../Development/appmesh/mesh.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -version="Pre Alpha" -comments="DEVELOPMENT"	rc version.rc
PostBuild_Cmds=..\tools\postbuild release\mesh Pandemic Mesh Viewer [Release]
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "appmesh - Win32 Release"
# Name "appmesh - Win32 Debug"
# Name "appmesh - Win32 Development"
# Begin Source File

SOURCE=.\guid.cpp
# End Source File
# Begin Source File

SOURCE=.\guid.h
# End Source File
# Begin Source File

SOURCE=..\graphics\meshmrm.cpp
# ADD CPP /D "INITGUID"
# End Source File
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "appmesh - Win32 Release"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "appmesh - Win32 Debug"

!ELSEIF  "$(CFG)" == "appmesh - Win32 Development"

# ADD BASE CPP /Yc"std.h"
# ADD CPP /Yc"std.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\version.RES
# End Source File
# Begin Source File

SOURCE=.\winmain.cpp
# End Source File
# End Target
# End Project
