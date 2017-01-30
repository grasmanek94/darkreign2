# Microsoft Developer Studio Project File - Name="CryptSmall" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CryptSmall - Win32 Debug_W95
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CryptSmall.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CryptSmall.mak" CFG="CryptSmall - Win32 Debug_W95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CryptSmall - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CryptSmall - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CryptSmall - Win32 Watson" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CryptSmall - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseSmall"
# PROP Intermediate_Dir "ReleaseSmall"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O1 /I ".." /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_WONCRYPT_NOEXCEPTIONS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\CryptSmall.lib"

!ELSEIF  "$(CFG)" == "CryptSmall - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugSmall"
# PROP Intermediate_Dir "DebugSmall"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".." /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /D "_WINDOWS" /D "WIN32" /D "_WONCRYPT_NOEXCEPTIONS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\CryptSmall_debug.lib"

!ELSEIF  "$(CFG)" == "CryptSmall - Win32 Watson"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CryptSmall___Win32_Watson"
# PROP BASE Intermediate_Dir "CryptSmall___Win32_Watson"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WatsonSmall"
# PROP Intermediate_Dir "WatsonSmall"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_DEBUG" /D "_NO_TOPLEVEL_CATCH" /D "_WINDOWS" /D "WIN32" /D "_WONCRYPT_NOEXCEPTIONS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\CryptSmall_watson.lib"

!ENDIF 

# Begin Target

# Name "CryptSmall - Win32 Release"
# Name "CryptSmall - Win32 Debug"
# Name "CryptSmall - Win32 Watson"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Crypto Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cryptoFiles\algebra.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\asn.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\bfinit.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\cbc.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\cryptlib.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\elgamal.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\eprecomp.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\filters.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\integer.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\iterhash.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\modes.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\modexppc.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\nbtheory.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\nr.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\pch.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\pubkey.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\queue.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\randpool.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\BFSymmetricKey.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptKeyBase.cpp
# End Source File
# Begin Source File

SOURCE=.\EGPrivateKey.cpp
# End Source File
# Begin Source File

SOURCE=.\EGPublicKey.cpp
# End Source File
# Begin Source File

SOURCE=.\MD5Hash.cpp
# End Source File
# Begin Source File

SOURCE=.\Randomizer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Crypto Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cryptoFiles\algebra.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\asn.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\cbc.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\config.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\cryptlib.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\elgamal.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\eprecomp.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\filters.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\integer.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\iterhash.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\md5.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\mdc.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\misc.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\modarith.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\modes.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\modexppc.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\nbtheory.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\nr.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\pch.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\pubkey.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\queue.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\randpool.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\smartptr.h
# End Source File
# Begin Source File

SOURCE=.\cryptoFiles\words.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\BFSymmetricKey.h
# End Source File
# Begin Source File

SOURCE=.\CryptException.h
# End Source File
# Begin Source File

SOURCE=.\CryptKeyBase.h
# End Source File
# Begin Source File

SOURCE=.\EGPrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\EGPublicKey.h
# End Source File
# Begin Source File

SOURCE=.\MD5Hash.h
# End Source File
# Begin Source File

SOURCE=.\PrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\PublicKey.h
# End Source File
# Begin Source File

SOURCE=.\Randomizer.h
# End Source File
# Begin Source File

SOURCE=.\SymmetricKey.h
# End Source File
# End Group
# End Target
# End Project
