# Microsoft Developer Studio Project File - Name="msg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=msg - Win32 Debug_W95
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msg.mak" CFG="msg - Win32 Debug_W95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "msg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "msg - Win32 Watson" (based on "Win32 (x86) Static Library")
!MESSAGE "msg - Win32 Watson_W95" (based on "Win32 (x86) Static Library")
!MESSAGE "msg - Win32 Release_W95" (based on "Win32 (x86) Static Library")
!MESSAGE "msg - Win32 Debug_W95" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Titan/lib/msg", UUOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msg - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\msg.lib"

!ELSEIF  "$(CFG)" == "msg - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\msg_debug.lib"

!ELSEIF  "$(CFG)" == "msg - Win32 Watson"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "msg"
# PROP BASE Intermediate_Dir "msg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Watson"
# PROP Intermediate_Dir "Watson"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\inc" /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\msg_debug.lib"
# ADD LIB32 /nologo /out:"..\bin\msg_watson.lib"

!ELSEIF  "$(CFG)" == "msg - Win32 Watson_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "msg"
# PROP BASE Intermediate_Dir "msg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Watson_W95"
# PROP Intermediate_Dir "Watson_W95"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\msg_watson.lib"
# ADD LIB32 /nologo /out:"..\bin\msg_W95_watson.lib"

!ELSEIF  "$(CFG)" == "msg - Win32 Release_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "message0"
# PROP BASE Intermediate_Dir "message0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_W95"
# PROP Intermediate_Dir "Release_W95"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\msg.lib"
# ADD LIB32 /nologo /out:"..\bin\msg_W95.lib"

!ELSEIF  "$(CFG)" == "msg - Win32 Debug_W95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "message1"
# PROP BASE Intermediate_Dir "message1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_W95"
# PROP Intermediate_Dir "Debug_W95"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\inc" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I ".." /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "WON_DEBUGENABLE" /D "WON_TRACEENABLE" /D "_NO_TOPLEVEL_CATCH" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\bin\msg_debug.lib"
# ADD LIB32 /nologo /out:"..\bin\msg_W95_debug.lib"

!ENDIF 

# Begin Target

# Name "msg - Win32 Release"
# Name "msg - Win32 Debug"
# Name "msg - Win32 Watson"
# Name "msg - Win32 Watson_W95"
# Name "msg - Win32 Release_W95"
# Name "msg - Win32 Debug_W95"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Common Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Comm\CommMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommGetNetStat.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommGetNumUsers.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommIsUserPresent.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommNoOp.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommPing.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommDumpStatistics.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommRegisterRequest.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommDebugLevel.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommNoOp.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommPing.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommQueryOptions.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommRehupOptions.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownConfirmation.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownConfirmationReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownRequest.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommTimeout.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommTracing.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Auth Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Auth\AuthMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Auth\MMsgAuth1KeepAliveHL.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\MMsgAuthRawBufferBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1GetPubKeys.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1Login.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1Login2.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginBase2.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginHL.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginHW.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginReplyHL.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1PeerToPeer.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuthLastRawBufferBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuthRawBufferBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Dir Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dir\DirEntity.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\DirMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Dir\DirMsgsG2.cpp
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2AddDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2AddService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ClearDataObjects.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2DumpTree.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2EntityBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2EntityListBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ExplicitSetDataObjects.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2FindDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2FindService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetDirectoryContents.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetEntity.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetNumEntities.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2KeyedBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2LoadFromSrc.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyDataObjects.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2MultiEntityReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2NameEntity.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerAttach.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerConnect.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerDataBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerSynch.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryExtendBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryFindBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryGetBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2RemoveEntity.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2RenewEntity.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SetDataObjects.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SingleEntityReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2StatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SynchTree.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2UpdateExtendBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirAddDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirAddService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirChangeDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirChangeService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirDirectoryBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirDumpTree.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirFindService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirGetDirContents.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirGetNumDirEntries.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirLoadFromSource.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerConnect.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerDataBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerSynch.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRemoveDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRemoveService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRenewDirectory.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRenewService.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirServiceBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirSynchDirTree.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Fact Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fact\FactMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactAutoStartRequest.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetAllProcesses.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetAllProcessesReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFileCRC.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFileCRCReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFreeDiskSpace.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFreeDiskSpaceReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessConfiguration.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessList.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessPorts.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetUsage.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetUsageReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactKillProcess.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactProcessConfigurationReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactProcessListReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactPullConfigFile.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactRunProcess.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactSaveConfigFile.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcess.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcessBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcessUnicode.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStopProcess.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStreamFile.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcess.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcessBase.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcessUnicode.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Ping Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ping\PingMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Ping\SMsgPingPingReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Ping\SMsgPingPingRequest.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "SIGSAuth"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SIGSAuth\SIGSAuthMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthAuthenticateUser.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthChangeUser.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthCreateUser.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Event Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Event\EventMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventRecordEvent.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventTaggedRecordEvent.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventTaggedStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Obs Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Obs\MMsgObsAddPublisher.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsAddSubscriber.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsBroadcastDataPool.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsBroadcastStream.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsCancelSubscription.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsDataPoolUpdated.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumeratePublications.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumeratePublishers.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumerateSubscribers.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumerateSubscriptions.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetDataPool.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetDataPoolSize.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetIdReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetName.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetNumSubscribers.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetPublicationId.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetPublisherId.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublicationSubscribedTo.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishDataPool.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishRequested.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishStream.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsReestablishPersistentTCP.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemovePublication.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemovePublisher.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemoveSubscriber.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSendStream.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeById.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeByName.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscriptionCanceled.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsUpdateDataPool.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Obs\ObsMsgs.cpp
# End Source File
# End Group
# Begin Group "Routing Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Routing\AddresseeList.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAcceptClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAddSuccessor.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAddToGroup.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBanClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBaseDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBootClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingClientBooted.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingClientChange.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCloseRegistration.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCreateDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCreateGroup.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDeleteDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDeleteGroup.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDisconnectClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetClientList.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetGroupList.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetSimpleClientList.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGroupChange.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGroupSpectatorCount.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingHostChange.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingHostSuccessionInProgress.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingInviteClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingKeepAlive.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingModifyDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingMuteClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingOpenRegistration.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerChat.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerData.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerDataMultiple.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReadDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReadDataObjectReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReconnectClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRegisterClient.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRemoveFromGroup.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRemoveSuccessor.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRenewDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReplaceDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendChat.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendData.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendDataBroadcast.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendDataMultiple.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSetPassword.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSpectatorCount.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSubscribeDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingUnsubscribeDataObject.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerClientFlagMessage.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerMessage.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Chat Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Chat\TMsgChat.cpp
# End Source File
# End Group
# Begin Group "Firewall Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Firewall\FirewallMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Firewall\SMsgFirewallDetect.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Firewall\SMsgFirewallStatusReply.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Contest Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Contest\MMsgContestDBCall.cpp
# End Source File
# Begin Source File

SOURCE=.\Contest\MMsgContestDBCall.h
# End Source File
# Begin Source File

SOURCE=.\Contest\MMsgTypesContest.h
# End Source File
# End Group
# Begin Group "Profile Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Profile\SMsgProfileMsgs.cpp
# End Source File
# End Group
# Begin Group "Par Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Par\ParMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Par\SMsgParStatusReply.cpp

!IF  "$(CFG)" == "msg - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "msg - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "msg - Win32 Watson"

!ELSEIF  "$(CFG)" == "msg - Win32 Watson_W95"

!ELSEIF  "$(CFG)" == "msg - Win32 Release_W95"

!ELSEIF  "$(CFG)" == "msg - Win32 Debug_W95"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\BadMsgException.cpp
# End Source File
# Begin Source File

SOURCE=.\tmessage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "Common Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Comm\MMsgCommGetNetStat.h
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommGetNumUsers.h
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommIsUserPresent.h
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommNoOp.h
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgCommPing.h
# End Source File
# Begin Source File

SOURCE=.\Comm\MMsgTypesComm.h
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommDumpStatistics.h
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommRegisterRequest.h
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgCommStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Comm\SMsgTypesComm.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommDebugLevel.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommNoOp.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommPing.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommQueryOptions.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommRehupOptions.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownConfirmation.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownConfirmationReply.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommShutDownRequest.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommTimeout.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgCommTracing.h
# End Source File
# Begin Source File

SOURCE=.\Comm\TMsgTypesComm.h
# End Source File
# End Group
# Begin Group "Auth Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Auth\MMsgAuth1KeepAliveHL.h
# End Source File
# Begin Source File

SOURCE=.\Auth\MMsgAuthRawBufferBase.h
# End Source File
# Begin Source File

SOURCE=.\Auth\MMsgTypesAuth.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1GetPubKeys.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1Login.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1Login2.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginBase2.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginHL.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginHW.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginReply.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1LoginReplyHL.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuth1PeerToPeer.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuthLastRawBufferBase.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgAuthRawBufferBase.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgProtocolTypes.h
# End Source File
# Begin Source File

SOURCE=.\Auth\TMsgTypesAuth.h
# End Source File
# End Group
# Begin Group "Dir Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dir\DirEntity.h
# End Source File
# Begin Source File

SOURCE=.\Dir\DirG2Flags.h
# End Source File
# Begin Source File

SOURCE=.\Dir\DirServerEntry.h
# End Source File
# Begin Source File

SOURCE=.\Dir\DirServerSKMasks.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2AddDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2AddService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ClearDataObjects.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2DumpTree.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2EntityBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2EntityListBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ExplicitSetDataObjects.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2FindDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2FindService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetDirectoryContents.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetEntity.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2GetNumEntities.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2KeyedBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2LoadFromSrc.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyDataObjects.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2ModifyService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2MultiEntityReply.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2NameEntity.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerAttach.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerConnect.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerDataBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2PeerSynch.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryExtendBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryFindBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2QueryGetBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2RemoveEntity.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2RenewEntity.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SetDataObjects.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SingleEntityReply.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2StatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2SynchTree.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2UpdateBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgDirG2UpdateExtendBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\SMsgTypesDir.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirAddDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirAddService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirChangeDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirChangeService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirDirectoryBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirDumpTree.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirFindService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirGetDirContents.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirGetNumDirEntries.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirLoadFromSource.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerConnect.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerDataBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirPeerSynch.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRemoveDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRemoveService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRenewDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirRenewService.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirServiceBase.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgDirSynchDirTree.h
# End Source File
# Begin Source File

SOURCE=.\Dir\TMsgTypesDir.h
# End Source File
# End Group
# Begin Group "Fact Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fact\AllMsgStartProcessBase.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactAutoStartRequest.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetAllProcesses.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetAllProcessesReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFileCRC.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFileCRCReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFreeDiskSpace.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetFreeDiskSpaceReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessConfiguration.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessList.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetProcessPorts.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetUsage.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactGetUsageReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactKillProcess.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactProcessConfigurationReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactProcessListReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactPullConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactRunProcess.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactSaveConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcess.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcessBase.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStartProcessUnicode.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStopProcess.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgFactStreamFile.h
# End Source File
# Begin Source File

SOURCE=.\Fact\SMsgTypesFact.h
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcess.h
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcessBase.h
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStartProcessUnicode.h
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgFactStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Fact\TMsgTypesFact.h
# End Source File
# End Group
# Begin Group "Ping Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ping\SMsgPingPingReply.h
# End Source File
# Begin Source File

SOURCE=.\Ping\SMsgPingPingRequest.h
# End Source File
# Begin Source File

SOURCE=.\Ping\SMsgTypesPing.h
# End Source File
# End Group
# Begin Group "SIGSAuth No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthAuthenticateUser.h
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthChangeUser.h
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthCreateUser.h
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgSIGSAuthStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\SIGSAuth\TMsgTypesSIGSAuth.h
# End Source File
# End Group
# Begin Group "Event Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Event\SMsgEventRecordEvent.h
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventTaggedRecordEvent.h
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgEventTaggedStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Event\SMsgTypesEvent.h
# End Source File
# End Group
# Begin Group "Obs Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Obs\MMsgObsAddPublisher.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsAddSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsBroadcastDataPool.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsBroadcastStream.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsCancelSubscription.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsDataPoolUpdated.H
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumeratePublications.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumeratePublishers.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumerateSubscribers.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsEnumerateSubscriptions.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetDataPool.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetDataPoolSize.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetIdReply.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetName.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetNumSubscribers.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetPublicationId.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsGetPublisherId.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublicationSubscribedTo.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishDataPool.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishReply.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishRequested.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsPublishStream.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsReestablishPersistentTCP.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemovePublication.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemovePublisher.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsRemoveSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSendStream.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeById.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeByName.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscribeReply.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsSubscriptionCanceled.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgObsUpdateDataPool.h
# End Source File
# Begin Source File

SOURCE=.\Obs\MMsgTypesObs.h
# End Source File
# Begin Source File

SOURCE=.\Obs\ObsServerListElements.h
# End Source File
# End Group
# Begin Group "Routing Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Routing\AddresseeList.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAcceptClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAddSuccessor.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingAddToGroup.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBanClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBaseDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingBootClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingClientBooted.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingClientChange.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCloseRegistration.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCreateDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingCreateGroup.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDeleteDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDeleteGroup.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingDisconnectClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetClientList.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetGroupList.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGetSimpleClientList.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGroupChange.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingGroupSpectatorCount.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingHostChange.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingHostSuccessionInProgress.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingInviteClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingKeepAlive.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingModifyDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingMuteClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingOpenRegistration.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerChat.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerData.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingPeerDataMultiple.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReadDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReadDataObjectReply.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReconnectClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRegisterClient.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRemoveFromGroup.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRemoveSuccessor.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingRenewDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingReplaceDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendChat.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendData.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendDataBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSendDataMultiple.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSetPassword.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSpectatorCount.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingSubscribeDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgRoutingUnsubscribeDataObject.h
# End Source File
# Begin Source File

SOURCE=.\Routing\MMsgTypesRouting.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerClientFlagMessage.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerMessage.h
# End Source File
# Begin Source File

SOURCE=.\Routing\RoutingServerTypes.h
# End Source File
# End Group
# Begin Group "Chat Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Chat\ChatTypes.h
# End Source File
# Begin Source File

SOURCE=.\Chat\TMsgChat.h
# End Source File
# End Group
# Begin Group "Firewall Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Firewall\SMsgFirewallDetect.h
# End Source File
# Begin Source File

SOURCE=.\Firewall\SMsgFirewallStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Firewall\SMsgTypesFirewall.h
# End Source File
# End Group
# Begin Group "Profile Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Profile\SMsgProfileMsgs.h
# End Source File
# Begin Source File

SOURCE=.\Profile\SMsgTypesProfile.h
# End Source File
# End Group
# Begin Group "Par Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\Par\SMsgParStatusReply.h
# End Source File
# Begin Source File

SOURCE=.\Par\SMsgTypesPar.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\BadMsgException.h
# End Source File
# Begin Source File

SOURCE=.\HeaderTypes.h
# End Source File
# Begin Source File

SOURCE=.\LServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\MServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\ServerStatus.h
# End Source File
# Begin Source File

SOURCE=.\SServiceTypes.h
# End Source File
# Begin Source File

SOURCE=.\tmessage.h
# End Source File
# Begin Source File

SOURCE=.\TServiceTypes.h
# End Source File
# End Group
# End Target
# End Project
