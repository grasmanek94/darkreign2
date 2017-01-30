# Microsoft Developer Studio Project File - Name="Observation" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Observation - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Observation.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Observation.mak" CFG="Observation - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Observation - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Observation - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Observation - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Observation - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\\" /I "..\msg\obs" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
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

# Name "Observation - Win32 Release"
# Name "Observation - Win32 Debug"
# Begin Group "Observation"

# PROP Default_Filter ""
# Begin Group "Source Files"

# PROP Default_Filter "*.c;*.cpp"
# Begin Source File

SOURCE=.\ObservationAPI.cpp

!IF  "$(CFG)" == "Observation - Win32 Release"

!ELSEIF  "$(CFG)" == "Observation - Win32 Debug"

# ADD CPP /I "..\..\msg\obs"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\msg\Obs\ObsMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\SocketMgr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\ObservationAPI.h
# End Source File
# Begin Source File

SOURCE=.\ObservationMessages.h
# End Source File
# Begin Source File

SOURCE=.\SocketMgr.h
# End Source File
# End Group
# End Group
# Begin Group "SDKCommon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SDKCommon\Completion.cpp
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\Completion.h
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\Event.h
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\MemStream.cpp
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\MemStream.h
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\Stream.cpp
# End Source File
# Begin Source File

SOURCE=..\SDKCommon\Stream.h
# End Source File
# End Group
# Begin Group "Socket"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Socket\IPSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\IPSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\IPXSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\IPXSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\PXSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\PXSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\Socket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\SPXSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\SPXSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\TCPSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\TCPSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\TMsgSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\TMsgSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\UDPSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\UDPSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket\Winsock.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\Winsock.h
# End Source File
# Begin Source File

SOURCE=..\Socket\WSSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket\WSSocket.h
# End Source File
# End Group
# Begin Group "msg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\msg\BadMsgException.cpp
# End Source File
# Begin Source File

SOURCE=..\msg\BadMsgException.h
# End Source File
# Begin Source File

SOURCE=..\msg\HeaderTypes.h
# End Source File
# Begin Source File

SOURCE=..\msg\LServiceTypes.h
# End Source File
# Begin Source File

SOURCE=..\msg\MServiceTypes.h
# End Source File
# Begin Source File

SOURCE=..\msg\ServerStatus.h
# End Source File
# Begin Source File

SOURCE=..\msg\SServiceTypes.h
# End Source File
# Begin Source File

SOURCE=..\msg\tmessage.cpp
# End Source File
# Begin Source File

SOURCE=..\msg\tmessage.h
# End Source File
# Begin Source File

SOURCE=..\msg\TServiceTypes.h
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\msg\Comm\CommMsgs.cpp
# End Source File
# Begin Source File

SOURCE=..\common\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=..\common\DataObject.cpp
# End Source File
# Begin Source File

SOURCE=..\common\DataObject.h
# End Source File
# Begin Source File

SOURCE=..\common\EventLog.cpp
# End Source File
# Begin Source File

SOURCE=..\common\EventLog.h
# End Source File
# Begin Source File

SOURCE=..\common\OutputOperators.h
# End Source File
# Begin Source File

SOURCE=..\common\RegKey.cpp
# End Source File
# Begin Source File

SOURCE=..\common\RegKey.h
# End Source File
# Begin Source File

SOURCE=..\common\Threadbase.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Threadbase.h
# End Source File
# Begin Source File

SOURCE=..\common\UTF8String.cpp
# End Source File
# Begin Source File

SOURCE=..\common\UTF8String.h
# End Source File
# Begin Source File

SOURCE=..\common\Utils.cpp
# End Source File
# Begin Source File

SOURCE=..\common\WON.h
# End Source File
# Begin Source File

SOURCE=..\common\WONDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\common\WONDebug.h
# End Source File
# Begin Source File

SOURCE=..\common\WONExceptCodes.h
# End Source File
# Begin Source File

SOURCE=..\common\WONException.cpp
# End Source File
# Begin Source File

SOURCE=..\common\WONException.h
# End Source File
# End Group
# End Target
# End Project
