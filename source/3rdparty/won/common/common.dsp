# Microsoft Developer Studio Project File - Name="common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=common - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "common.mak" CFG="common - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "common - Win32 Watson" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Titan/lib/common", VKOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "common - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\common.lib"

!ELSEIF  "$(CFG)" == "common - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\common_debug.lib"

!ELSEIF  "$(CFG)" == "common - Win32 Watson"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "common__"
# PROP BASE Intermediate_Dir "common__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Watson"
# PROP Intermediate_Dir "Watson"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /D _WIN32_WINNT=0x0400 /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_DEBUG" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\common_debug.lib"
# ADD LIB32 /nologo /out:"..\bin\common_watson.lib"

!ENDIF 

# Begin Target

# Name "common - Win32 Release"
# Name "common - Win32 Debug"
# Name "common - Win32 Watson"
# Begin Group "Source Files"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\CRC16.cpp
# End Source File
# Begin Source File

SOURCE=.\CRC32.cpp
# End Source File
# Begin Source File

SOURCE=.\DataObject.cpp

!IF  "$(CFG)" == "common - Win32 Release"

!ELSEIF  "$(CFG)" == "common - Win32 Debug"

# ADD CPP /I "..\lib"

!ELSEIF  "$(CFG)" == "common - Win32 Watson"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EventLog.cpp
# End Source File
# Begin Source File

SOURCE=.\RawList.cpp
# End Source File
# Begin Source File

SOURCE=.\RegKey.cpp
# End Source File
# Begin Source File

SOURCE=.\Threadbase.cpp
# End Source File
# Begin Source File

SOURCE=.\UTF8String.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\WONDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\WONException.cpp
# End Source File
# Begin Source File

SOURCE=.\WONString.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\array_auto_ptr.h
# End Source File
# Begin Source File

SOURCE=.\CRC16.h
# End Source File
# Begin Source File

SOURCE=.\CRC32.h
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\DataObject.h
# End Source File
# Begin Source File

SOURCE=.\EventLog.h
# End Source File
# Begin Source File

SOURCE=.\OutputOperators.h
# End Source File
# Begin Source File

SOURCE=.\ParameterDataTypes.h
# End Source File
# Begin Source File

SOURCE=.\RawList.h
# End Source File
# Begin Source File

SOURCE=.\RegKey.h
# End Source File
# Begin Source File

SOURCE=.\Threadbase.h
# End Source File
# Begin Source File

SOURCE=.\UTF8String.h
# End Source File
# Begin Source File

SOURCE=.\WON.h
# End Source File
# Begin Source File

SOURCE=.\WONDebug.h
# End Source File
# Begin Source File

SOURCE=.\WONExceptCodes.h
# End Source File
# Begin Source File

SOURCE=.\WONException.h
# End Source File
# End Group
# End Target
# End Project
