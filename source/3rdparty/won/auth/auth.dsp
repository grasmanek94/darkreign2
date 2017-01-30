# Microsoft Developer Studio Project File - Name="auth" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=auth - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "auth.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "auth.mak" CFG="auth - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "auth - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "auth - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "auth - Win32 Watson" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Titan/lib/auth", RVUAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "auth - Win32 Release"

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
# ADD LIB32 /nologo /out:"..\bin\auth.lib"

!ELSEIF  "$(CFG)" == "auth - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D "WIN32" /D _WIN32_WINNT=0x0400 /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\auth_debug.lib"

!ELSEIF  "$(CFG)" == "auth - Win32 Watson"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Watson"
# PROP BASE Intermediate_Dir "Watson"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Watson"
# PROP Intermediate_Dir "Watson"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_DEBUG" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\auth.lib"
# ADD LIB32 /nologo /out:"..\bin\auth_watson.lib"

!ENDIF 

# Begin Target

# Name "auth - Win32 Release"
# Name "auth - Win32 Debug"
# Name "auth - Win32 Watson"
# Begin Group "Source Files"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Auth1Certificate.cpp
# End Source File
# Begin Source File

SOURCE=.\Auth1PublicKeyBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\Auth2Certificate.cpp
# End Source File
# Begin Source File

SOURCE=.\AuthCertificateBase.cpp
# End Source File
# Begin Source File

SOURCE=.\AuthFamilyBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\AuthPublicKeyBlockBase.cpp
# End Source File
# Begin Source File

SOURCE=.\Session.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Auth1Certificate.h
# End Source File
# Begin Source File

SOURCE=.\Auth1Container.h
# End Source File
# Begin Source File

SOURCE=.\Auth1PublicKeyBlock.h
# End Source File
# Begin Source File

SOURCE=.\Auth2Certificate.h
# End Source File
# Begin Source File

SOURCE=.\AuthCertificateBase.h
# End Source File
# Begin Source File

SOURCE=.\AuthFamilies.h
# End Source File
# Begin Source File

SOURCE=.\AuthFamilyBuffer.h
# End Source File
# Begin Source File

SOURCE=.\AuthPublicKeyBlockBase.h
# End Source File
# Begin Source File

SOURCE=.\CryptFlags.h
# End Source File
# Begin Source File

SOURCE=.\Permission.h
# End Source File
# Begin Source File

SOURCE=.\Session.h
# End Source File
# End Group
# End Target
# End Project
