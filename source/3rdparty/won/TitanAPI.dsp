# Microsoft Developer Studio Project File - Name="TitanAPI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TitanAPI - Win32 Debug Internal Use
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TitanAPI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TitanAPI.mak" CFG="TitanAPI - Win32 Debug Internal Use"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TitanAPI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Debug Singlethreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Release Singlethreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Release Internal Use" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Debug Internal Use" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Release for MFC DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "TitanAPI - Win32 Debug for MFC DLL" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Titan/API", FHFBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

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
# ADD CPP /nologo /Gr /MT /W3 /GR /GX /Zi /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

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
# ADD CPP /nologo /Gr /MT /W3 /GR /GX /Zi /Od /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TitanAPI___Win32_Debug_Singlethreaded"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Debug_Singlethreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Singlethreaded"
# PROP Intermediate_Dir "Debug_Singlethreaded"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TitanAPI___Win32_Release_Singlethreaded"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Release_Singlethreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Singlethreaded"
# PROP Intermediate_Dir "Release_Singlethreaded"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TitanAPI___Win32_Debug_Singlethreaded_For_Multithreaded_Apps"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Debug_Singlethreaded_For_Multithreaded_Apps"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Singlethreaded_MT"
# PROP Intermediate_Dir "Debug_Singlethreaded_MT"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TitanAPI___Win32_Release_Singlethreaded_For_Multithreaded_Apps"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Release_Singlethreaded_For_Multithreaded_Apps"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Singlethreaded_MT"
# PROP Intermediate_Dir "Release_Singlethreaded_MT"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TitanAPI___Win32_Release_Internal_Use"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Release_Internal_Use"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Internal_Use"
# PROP Intermediate_Dir "Release_Internal_Use"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O1 /I "..\lib" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O1 /I "..\lib" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release_Internal_Use2\TitanAPI_internal.lib"
# ADD LIB32 /nologo /out:"Release_Internal_Use\TitanAPI_internal.lib"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TitanAPI___Win32_Debug_Internal_Use"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Debug_Internal_Use"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Internal_Use"
# PROP Intermediate_Dir "Debug_Internal_Use"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\lib" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\lib" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug_Internal_Use2\TitanAPI_internal_debug.lib"
# ADD LIB32 /nologo /out:"Debug_Internal_Use\TitanAPI_internal_debug.lib"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TitanAPI___Win32_Release_for_MFC_DLL"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Release_for_MFC_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TitanAPI___Win32_Release_for_MFC_DLL"
# PROP Intermediate_Dir "TitanAPI___Win32_Release_for_MFC_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TitanAPI___Win32_Debug_for_MFC_DLL0"
# PROP BASE Intermediate_Dir "TitanAPI___Win32_Debug_for_MFC_DLL0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TitanAPI___Win32_Debug_for_MFC_DLL"
# PROP Intermediate_Dir "TitanAPI___Win32_Debug_for_MFC_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /O1 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MTAPI" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TitanAPI - Win32 Release"
# Name "TitanAPI - Win32 Debug"
# Name "TitanAPI - Win32 Debug Singlethreaded"
# Name "TitanAPI - Win32 Release Singlethreaded"
# Name "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"
# Name "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"
# Name "TitanAPI - Win32 Release Internal Use"
# Name "TitanAPI - Win32 Debug Internal Use"
# Name "TitanAPI - Win32 Release for MFC DLL"
# Name "TitanAPI - Win32 Debug for MFC DLL"
# Begin Group "Authentication"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Authentication\AuthSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Authentication\AuthSocket.h
# End Source File
# Begin Source File

SOURCE=.\Authentication\ClientCDKey.cpp
# End Source File
# Begin Source File

SOURCE=.\Authentication\ClientCDKey.h
# End Source File
# Begin Source File

SOURCE=.\Authentication\Identity.cpp
# End Source File
# Begin Source File

SOURCE=.\Authentication\Identity.h
# End Source File
# Begin Source File

SOURCE=.\Authentication\Md52.cpp
# End Source File
# Begin Source File

SOURCE=.\Authentication\Md52.h
# End Source File
# End Group
# Begin Group "SDKCommon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SDKCommon\Completion.cpp
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\Completion.h
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\MemStream.cpp
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\MemStream.h
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\Pool.h
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\SDKCommon\Stream.h
# End Source File
# End Group
# Begin Group "Socket"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Socket\IPSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\IPSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\IPXSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\IPXSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\PXSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\PXSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\RawIPSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\RawIPSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\SPXSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\SPXSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\TCPSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\TCPSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\TMsgSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\TMsgSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\UDPSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\UDPSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\WONSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\WONSocket.h
# End Source File
# Begin Source File

SOURCE=.\Socket\WONWS.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\WONWS.h
# End Source File
# Begin Source File

SOURCE=.\Socket\WSSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket\WSSocket.h
# End Source File
# End Group
# Begin Group "auth"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\auth\Auth1Certificate.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\Auth1Certificate.h
# End Source File
# Begin Source File

SOURCE=.\auth\Auth1Container.h
# End Source File
# Begin Source File

SOURCE=.\auth\Auth1PrivateKeyBlock.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\Auth1PrivateKeyBlock.h
# End Source File
# Begin Source File

SOURCE=.\auth\Auth1PublicKeyBlock.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\Auth1PublicKeyBlock.h
# End Source File
# Begin Source File

SOURCE=.\auth\Auth2Certificate.cpp
# End Source File
# Begin Source File

SOURCE=.\auth\Auth2Certificate.h
# End Source File
# Begin Source File

SOURCE=.\auth\AuthCertificateBase.cpp
# End Source File
# Begin Source File

SOURCE=.\auth\AuthCertificateBase.h
# End Source File
# Begin Source File

SOURCE=.\auth\AuthFamilies.h
# End Source File
# Begin Source File

SOURCE=.\auth\AuthFamilyBuffer.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\AuthFamilyBuffer.h
# End Source File
# Begin Source File

SOURCE=.\msg\Auth\AuthMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\AuthPublicKeyBlockBase.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\AuthPublicKeyBlockBase.h
# End Source File
# Begin Source File

SOURCE=.\auth\CryptFlags.h
# End Source File
# Begin Source File

SOURCE=.\auth\Permission.h
# End Source File
# Begin Source File

SOURCE=.\auth\Session.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\auth\Session.h
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common\auto_obj.cpp
# End Source File
# Begin Source File

SOURCE=.\common\auto_obj.h
# End Source File
# Begin Source File

SOURCE=.\msg\Comm\CommMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\CRC16.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\CRC16.h
# End Source File
# Begin Source File

SOURCE=.\common\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\common\DataObject.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\DataObject.h
# End Source File
# Begin Source File

SOURCE=.\common\EventLog.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\EventLog.h
# End Source File
# Begin Source File

SOURCE=.\common\linuxGlue.h
# End Source File
# Begin Source File

SOURCE=.\common\macGlue.h
# End Source File
# Begin Source File

SOURCE=.\common\OutputOperators.h
# End Source File
# Begin Source File

SOURCE=.\common\ParameterDataTypes.h
# End Source File
# Begin Source File

SOURCE=.\common\RawList.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\RawList.h
# End Source File
# Begin Source File

SOURCE=.\common\RegKey.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\RegKey.h
# End Source File
# Begin Source File

SOURCE=.\common\Threadbase.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\Threadbase.h
# End Source File
# Begin Source File

SOURCE=.\common\UTF8String.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\UTF8String.h
# End Source File
# Begin Source File

SOURCE=.\common\Utils.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\WON.h
# End Source File
# Begin Source File

SOURCE=.\common\WONDebug.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\WONDebug.h
# End Source File
# Begin Source File

SOURCE=.\common\WONExceptCodes.h
# End Source File
# Begin Source File

SOURCE=.\common\WONException.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\WONException.h
# End Source File
# Begin Source File

SOURCE=.\common\WONString.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\WONString.h
# End Source File
# End Group
# Begin Group "msg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\msg\BadMsgException.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msg\BadMsgException.h
# End Source File
# Begin Source File

SOURCE=.\msg\HeaderTypes.h
# End Source File
# Begin Source File

SOURCE=.\msg\LServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\msg\MServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\msg\ServerStatus.h
# End Source File
# Begin Source File

SOURCE=.\msg\SServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\msg\tmessage.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msg\tmessage.h
# End Source File
# Begin Source File

SOURCE=.\msg\TServiceTypes.h
# End Source File
# End Group
# Begin Group "Routing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Routing\CRoutingAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Routing\CRoutingAPI.h
# End Source File
# Begin Source File

SOURCE=.\Routing\CRoutingAPITypes.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingAPI.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingAPITypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingAPITypes.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingMessages.h
# End Source File
# Begin Source File

SOURCE=.\msg\Routing\RoutingMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerDirEntity.cpp
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerDirEntity.h
# End Source File
# End Group
# Begin Group "Firewall"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Firewall\FirewallAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Firewall\FirewallAPI.h
# End Source File
# Begin Source File

SOURCE=.\msg\Firewall\FirewallMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# End Group
# Begin Group "Factory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\msg\Fact\FactMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Factory\FactoryAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Factory\FactoryAPI.h
# End Source File
# End Group
# Begin Group "Directory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Directory\DirectoryAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Directory\DirectoryAPI.h
# End Source File
# Begin Source File

SOURCE=.\msg\Dir\DirMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msg\Dir\DirMsgsG2.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Directory\Legacy.cpp
# End Source File
# End Group
# Begin Group "Observation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Observation\ObservationAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Observation\ObservationAPI.h
# End Source File
# Begin Source File

SOURCE=.\Observation\ObservationMessages.h
# End Source File
# Begin Source File

SOURCE=.\msg\Obs\ObsMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Observation\SocketMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Observation\SocketMgr.h
# End Source File
# End Group
# Begin Group "Contest"

# PROP Default_Filter ""
# Begin Group "Internal - Prizecentral"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Contest\Prizecentral.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Contest\Prizecentral.h
# End Source File
# Begin Source File

SOURCE=.\msg\DB\SMsgDBRegisterUser.cpp
# End Source File
# Begin Source File

SOURCE=.\msg\DB\SMsgDBRegisterUser.h
# End Source File
# Begin Source File

SOURCE=.\msg\DB\SMsgDBRegisterUserReply.cpp
# End Source File
# Begin Source File

SOURCE=.\msg\DB\SMsgDBRegisterUserReply.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Contest\ContestAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Contest\ContestAPI.h
# End Source File
# Begin Source File

SOURCE=.\msg\Contest\MMsgContestDBCall.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# End Group
# Begin Group "Profile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Profile\Profile2API.cpp
# End Source File
# Begin Source File

SOURCE=.\Profile\Profile2API.h
# End Source File
# Begin Source File

SOURCE=.\Profile\ProfileAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Profile\ProfileAPI.h
# End Source File
# Begin Source File

SOURCE=.\msg\Profile\SMsgProfileMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\msg\Profile\SMsgProfileMsgs.h
# End Source File
# Begin Source File

SOURCE=.\msg\Profile\SMsgTypesProfile.h
# End Source File
# End Group
# Begin Group "Event"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Event\EventAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Event\EventAPI.h
# End Source File
# Begin Source File

SOURCE=.\msg\Event\EventMsgs.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# End Group
# Begin Group "crypt"

# PROP Default_Filter ""
# Begin Group "cryptoFiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crypt\cryptoFiles\algebra.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\algebra.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\asn.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\asn.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\bfinit.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\blowfish.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\cbc.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\cbc.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\config.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\cryptlib.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\cryptlib.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\elgamal.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\elgamal.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\eprecomp.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\eprecomp.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\filters.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\filters.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\integer.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\integer.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\iterhash.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\iterhash.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\md5.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\md5.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\mdc.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\misc.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\misc.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\modarith.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\modes.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\modes.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\modexppc.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\modexppc.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\nbtheory.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\nbtheory.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\nr.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\nr.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\pch.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\pch.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\pubkey.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\pubkey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\queue.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\queue.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\randpool.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\randpool.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\smartptr.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptoFiles\words.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\crypt\BFSymmetricKey.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\BFSymmetricKey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\BigInteger.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\BigInteger.h
# End Source File
# Begin Source File

SOURCE=.\crypt\BigNum.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\BigNum.h
# End Source File
# Begin Source File

SOURCE=.\crypt\CryptException.h
# End Source File
# Begin Source File

SOURCE=.\crypt\CryptFlags.h
# End Source File
# Begin Source File

SOURCE=.\crypt\CryptKeyBase.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\CryptKeyBase.h
# End Source File
# Begin Source File

SOURCE=.\crypt\EGPrivateKey.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\EGPrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\EGPublicKey.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\EGPublicKey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\IntegerExtractor.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\IntegerExtractor.h
# End Source File
# Begin Source File

SOURCE=.\crypt\IntegerInserter.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\IntegerInserter.h
# End Source File
# Begin Source File

SOURCE=.\crypt\MD5Digest.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\MD5Hash.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\MD5Hash.h
# End Source File
# Begin Source File

SOURCE=.\crypt\MiniBlowfish.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\MiniBlowfish.h
# End Source File
# Begin Source File

SOURCE=.\crypt\MiniElGamal.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\MiniElGamal.h
# End Source File
# Begin Source File

SOURCE=.\crypt\MiniMisc.h
# End Source File
# Begin Source File

SOURCE=.\crypt\PrivateKey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\PublicKey.h
# End Source File
# Begin Source File

SOURCE=.\crypt\Random.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\Random.h
# End Source File
# Begin Source File

SOURCE=.\crypt\Randomizer.cpp

!IF  "$(CFG)" == "TitanAPI - Win32 Release"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Singlethreaded For Multithreaded Apps"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug Internal Use"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Release for MFC DLL"

!ELSEIF  "$(CFG)" == "TitanAPI - Win32 Debug for MFC DLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\Randomizer.h
# End Source File
# Begin Source File

SOURCE=.\crypt\SymmetricKey.h
# End Source File
# End Group
# Begin Group "UserSearch"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UserSearch\UserSearchCounts.cpp
# End Source File
# Begin Source File

SOURCE=.\UserSearch\UserSearchCounts.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Errors.cpp
# End Source File
# Begin Source File

SOURCE=.\Errors.h
# End Source File
# Begin Source File

SOURCE=.\Ping.cpp
# End Source File
# Begin Source File

SOURCE=.\Ping.h
# End Source File
# Begin Source File

SOURCE=.\wondll.h
# End Source File
# Begin Source File

SOURCE=.\WONHTTP.cpp
# End Source File
# Begin Source File

SOURCE=.\WONHTTP.h
# End Source File
# End Target
# End Project
