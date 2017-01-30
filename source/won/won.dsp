# Microsoft Developer Studio Project File - Name="won" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=won - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "won.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "won.mak" CFG="won - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "won - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "won - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "won - Win32 Development" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/won", MPCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "won - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/won"
# PROP Intermediate_Dir "../Release/won"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WON_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /O2 /Oy- /I "../3rdparty" /I "../3rdparty/won" /I "../system" /I "../util" /D "WIN32" /D "NDEBUG" /D "_USRDLL" /D "_MBCS" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 user32.lib gdi32.lib /nologo /dll /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -company="Pandemic Studios" -description="WON Interface" -version="Pre Alpha" -comments="RELEASE"	rc version.rc
PostBuild_Cmds=..\tools\postdll ..\release\won\won
# End Special Build Tool

!ELSEIF  "$(CFG)" == "won - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "won___Win32_Debug"
# PROP BASE Intermediate_Dir "won___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/won"
# PROP Intermediate_Dir "../Debug/won"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W4 /GX /Zi /Od /I "../3rdparty" /I "../3rdparty/won" /I "../system" /I "../util" /FI"std.h" /D "_DEBUG" /D "DEBUG" /D "DEVELOPMENT" /D "_DLL" /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../3rdparty/won" /I "../system" /I "../util" /D "_USRDLL" /D "_MBCS" /D "WIN32" /D "DEVELOPMENT" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /incremental:no /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib /nologo /dll /incremental:no /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -company="Pandemic Studios" -description="WON Interface" -version="Pre Alpha" -comments="DEBUG"	rc version.rc
PostBuild_Cmds=..\tools\postdll ..\debug\won\won
# End Special Build Tool

!ELSEIF  "$(CFG)" == "won - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "won___Win32_Development"
# PROP BASE Intermediate_Dir "won___Win32_Development"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/won"
# PROP Intermediate_Dir "../Development/won"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /MT /W4 /GX /O2 /I "../3rdparty" /I "../3rdparty/won" /I "../system" /I "../util" /FI"std.h" /D "_USRDLL" /D "_MBCS" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /O2 /Oy- /I "../3rdparty" /I "../3rdparty/won" /I "../system" /I "../util" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /D "_USRDLL" /D "_MBCS" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib /nologo /dll /machine:I386
# ADD LINK32 user32.lib gdi32.lib /nologo /dll /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=..\tools\version version.rc -company="Pandemic Studios" -description="WON Interface" -version="Pre Alpha" -comments="DEVELOPMENT"	rc version.rc
PostBuild_Cmds=..\tools\postdll ..\development\won\won
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "won - Win32 Release"
# Name "won - Win32 Debug"
# Name "won - Win32 Development"
# Begin Source File

SOURCE=.\version.res
# End Source File
# Begin Source File

SOURCE=.\woniface.cpp
# End Source File
# Begin Source File

SOURCE=.\woniface.h
# End Source File
# End Target
# End Project
