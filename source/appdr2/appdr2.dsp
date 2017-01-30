# Microsoft Developer Studio Project File - Name="appdr2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=appdr2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "appdr2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "appdr2.mak" CFG="appdr2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "appdr2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "appdr2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "appdr2 - Win32 Development" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/appdr2", DAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "appdr2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/appdr2"
# PROP Intermediate_Dir "../Release/appdr2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 user32.lib gdi32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"../Release/appdr2/dr2.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -comments="RELEASE" -version=1,1	rc version.rc
PostBuild_Cmds=..\tools\postbuild ..\release\appdr2\dr2 Dark Reign II [Release]
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appdr2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/appdr2"
# PROP Intermediate_Dir "../Debug/appdr2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"../Debug/appdr2/dr2.exe" /pdbtype:sept
# SUBTRACT LINK32 /verbose /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -comments="DEBUG" -version=1,1	rc version.rc
PostBuild_Cmds=..\tools\postbuild ..\debug\appdr2\dr2 Dark Reign II [Debug]
# End Special Build Tool

!ELSEIF  "$(CFG)" == "appdr2 - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "appdr2___Win32_Development"
# PROP BASE Intermediate_Dir "appdr2___Win32_Development"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/appdr2"
# PROP Intermediate_Dir "../Development/appdr2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../Release/appdr2/dr2.exe"
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 user32.lib gdi32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../Development/appdr2/dr2.exe"
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -include=dr2.rc -company="Pandemic Studios" -description="Dark Reign 2" -comments="DEVELOPMENT" -version=1,1	rc version.rc
PostBuild_Cmds=..\tools\postbuild ..\development\appdr2\dr2 Dark Reign II [Development]
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "appdr2 - Win32 Release"
# Name "appdr2 - Win32 Debug"
# Name "appdr2 - Win32 Development"
# Begin Source File

SOURCE=.\dr2.ico
# End Source File
# Begin Source File

SOURCE=..\graphics\meshmrm.cpp
# End Source File
# Begin Source File

SOURCE=..\settings.h
# End Source File
# Begin Source File

SOURCE=.\version.res
# End Source File
# Begin Source File

SOURCE=.\winmain.cpp
# End Source File
# End Target
# End Project
