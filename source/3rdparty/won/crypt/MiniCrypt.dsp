# Microsoft Developer Studio Project File - Name="MiniCrypt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MiniCrypt - Win32 Release_W95
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MiniCrypt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MiniCrypt.mak" CFG="MiniCrypt - Win32 Release_W95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MiniCrypt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MiniCrypt - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MiniCrypt - Win32 Watson" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MiniCrypt - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Gz /MT /W3 /vd0 /O1 /I "..\." /D "NDEBUG" /D _WIN32_WINNT=0x0400 /D "_WINDOWS" /D "WIN32" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\MiniCrypt.lib"

!ELSEIF  "$(CFG)" == "MiniCrypt - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /vd0 /ZI /Od /I "..\." /D _WIN32_WINNT=0x0400 /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\MiniCrypt_debug.lib"

!ELSEIF  "$(CFG)" == "MiniCrypt - Win32 Watson"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Watson"
# PROP BASE Intermediate_Dir "Watson"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Watson"
# PROP Intermediate_Dir "Watson"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\." /D _WIN32_WINNT=0x0400 /D "_DEBUG" /D "_NO_TOPLEVEL_CATCH" /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\MiniCrypt_watson.lib"

!ENDIF 

# Begin Target

# Name "MiniCrypt - Win32 Release"
# Name "MiniCrypt - Win32 Debug"
# Name "MiniCrypt - Win32 Watson"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BigInteger.cpp
# End Source File
# Begin Source File

SOURCE=.\BigNum.cpp
# End Source File
# Begin Source File

SOURCE=.\Blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\ElGamal.cpp
# End Source File
# Begin Source File

SOURCE=.\IntegerExtractor.cpp
# End Source File
# Begin Source File

SOURCE=.\IntegerInserter.cpp
# End Source File
# Begin Source File

SOURCE=.\MD5Digest.cpp
# End Source File
# Begin Source File

SOURCE=.\Random.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BigInteger.h
# End Source File
# Begin Source File

SOURCE=.\BigNum.h
# End Source File
# Begin Source File

SOURCE=.\Blowfish.h
# End Source File
# Begin Source File

SOURCE=.\ElGamal.h
# End Source File
# Begin Source File

SOURCE=.\IntegerExtractor.h
# End Source File
# Begin Source File

SOURCE=.\IntegerInserter.h
# End Source File
# Begin Source File

SOURCE=.\MD5Digest.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\Random.h
# End Source File
# End Group
# Begin Group "Crypt Source"

# PROP Default_Filter ""
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
# End Group
# Begin Group "Crypt Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BFSymmetricKey.h
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

SOURCE=.\PrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\PublicKey.h
# End Source File
# Begin Source File

SOURCE=.\SymmetricKey.h
# End Source File
# End Group
# End Target
# End Project
