# Microsoft Developer Studio Project File - Name="WONCrypt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WONCrypt - Win32 Release_W95
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WONCrypt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WONCrypt.mak" CFG="WONCrypt - Win32 Release_W95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WONCrypt - Win32 Debug_W95" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WONCrypt - Win32 Release_W95" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WONCrypt - Win32 Debug_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WONCrypt"
# PROP BASE Intermediate_Dir "WONCrypt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WONCrypt_Debug_W95"
# PROP Intermediate_Dir "WONCrypt_Debug_W95"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"WONCrypt_Debug_W95/WONCr_W95_debug.dll" /pdbtype:sept /libpath:"..\bin"

!ELSEIF  "$(CFG)" == "WONCrypt - Win32 Release_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WONCrypt"
# PROP BASE Intermediate_Dir "WONCrypt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WONCrypt_Release_W95"
# PROP Intermediate_Dir "WONCrypt_Release_W95"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O1 /I ".." /D "_WINDOWS" /D "WIN32" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\bin"
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"WONCrypt_Release_W95/WONCr_W95.dll" /pdbtype:sept /libpath:"..\bin"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "WONCrypt - Win32 Debug_W95"
# Name "WONCrypt - Win32 Release_W95"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WON_BFSymmetricKey.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_CryptFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_CryptKeyBase.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_EGPrivateKey.cpp
# End Source File
# Begin Source File

SOURCE=.\WON_EGPublicKey.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WON_BFSymmetricKey.h
# End Source File
# Begin Source File

SOURCE=.\WON_CryptFactory.h
# End Source File
# Begin Source File

SOURCE=.\WON_CryptKeyBase.h
# End Source File
# Begin Source File

SOURCE=.\WON_EGPrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\WON_EGPublicKey.h
# End Source File
# End Group
# End Target
# End Project
