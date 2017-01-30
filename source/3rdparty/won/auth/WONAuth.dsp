# Microsoft Developer Studio Project File - Name="WONAuth" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WONAuth - Win32 Debug_W95
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WONAuth.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WONAuth.mak" CFG="WONAuth - Win32 Debug_W95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WONAuth - Win32 Release_W95" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WONAuth - Win32 Debug_W95" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WONAuth - Win32 Release_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_W95"
# PROP BASE Intermediate_Dir "Release_W95"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WONAuth_Release_W95"
# PROP Intermediate_Dir "WONAuth_Release_W95"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\lib" /I ".." /D "_WINDOWS" /D "WIN32" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /out:"WONAuth_Release_W95/WONAu_W95.dll"

!ELSEIF  "$(CFG)" == "WONAuth - Win32 Debug_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_W95"
# PROP BASE Intermediate_Dir "Debug_W95"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WONAuth_Debug_W95"
# PROP Intermediate_Dir "WONAuth_Debug_W95"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"WONAuth_Debug_W95/WONAu_W95_debug.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "WONAuth - Win32 Release_W95"
# Name "WONAuth - Win32 Debug_W95"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WON_AuthCertificate1.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_AuthFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_AuthFamilyBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_AuthPublicKeyBlock1.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WON_AuthCertificate1.h
# End Source File
# Begin Source File

SOURCE=.\WON_AuthFactory.h
# End Source File
# Begin Source File

SOURCE=.\WON_AuthFamilyBuffer.h
# End Source File
# Begin Source File

SOURCE=.\WON_AuthPublicKeyBlock1.h
# End Source File
# End Group
# End Target
# End Project
