# Microsoft Developer Studio Project File - Name="appvid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=appvid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "appvid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "appvid.mak" CFG="appvid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "appvid - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "appvid - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/dr2/code/appvid", MEAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "appvid - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MD /W4 /GX /O2 /Oy- /Ob2 /I "../3rdparty" /I "../coregame" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /FAcs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /pdb:none /map /machine:I386 /out:"Release/vid.exe"

!ELSEIF  "$(CFG)" == "appvid - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MDd /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "DEBUG" /D "_DEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib comdlg32.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /out:"Debug/vid.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "appvid - Win32 Release"
# Name "appvid - Win32 Debug"
# Begin Source File

SOURCE=.\appvid.cpp

!IF  "$(CFG)" == "appvid - Win32 Release"

!ELSEIF  "$(CFG)" == "appvid - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\appvid.h
# End Source File
# Begin Source File

SOURCE=.\appvid.rc
# End Source File
# Begin Source File

SOURCE=..\appmesh\guid.cpp

!IF  "$(CFG)" == "appvid - Win32 Release"

!ELSEIF  "$(CFG)" == "appvid - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\appmesh\guid.h
# End Source File
# Begin Source File

SOURCE=.\makeball.cpp

!IF  "$(CFG)" == "appvid - Win32 Release"

!ELSEIF  "$(CFG)" == "appvid - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\winmain.cpp

!IF  "$(CFG)" == "appvid - Win32 Release"

!ELSEIF  "$(CFG)" == "appvid - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
