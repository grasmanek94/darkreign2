# Microsoft Developer Studio Project File - Name="styxnet" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=styxnet - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "styxnet.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "styxnet.mak" CFG="styxnet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "styxnet - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "styxnet - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "styxnet - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/styxnet", BNFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "styxnet - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/styxnet"
# PROP Intermediate_Dir "../Release/styxnet"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /O2 /Oy- /I "../3rdparty" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "styxnet - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/StyxNet"
# PROP Intermediate_Dir "../Debug/StyxNet"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /Gm /GX /Zi /Od /I "../3rdparty" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /Yu"std.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "styxnet - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "styxnet___Win32_Development"
# PROP BASE Intermediate_Dir "styxnet___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/styxnet"
# PROP Intermediate_Dir "../Development/styxnet"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /MD /W4 /GX /O2 /I "../3rdparty" /I "../system" /I "../util" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /O2 /Oy- /I "../3rdparty" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "styxnet - Win32 Release"
# Name "styxnet - Win32 Debug"
# Name "styxnet - Win32 Development"
# Begin Group "client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\styxnet_client.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_client.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_client_private.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_client_session.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_client_user.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_clientmessage.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_clientresponse.h
# End Source File
# End Group
# Begin Group "explorer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\styxnet_explorer.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_explorer.h
# End Source File
# End Group
# Begin Group "internals"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\styxnet.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_data.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_eventqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_eventqueue.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_packet.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_packet.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_private.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_std.cpp
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\network_http.cpp
# End Source File
# Begin Source File

SOURCE=.\network_http.h
# End Source File
# Begin Source File

SOURCE=.\network_icmp.h
# End Source File
# Begin Source File

SOURCE=.\network_ip.cpp
# End Source File
# Begin Source File

SOURCE=.\network_ip.h
# End Source File
# Begin Source File

SOURCE=.\network_ping.cpp
# End Source File
# Begin Source File

SOURCE=.\network_ping.h
# End Source File
# Begin Source File

SOURCE=.\network_url.cpp
# End Source File
# Begin Source File

SOURCE=.\network_url.h
# End Source File
# End Group
# Begin Group "server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\styxnet_server.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_server.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_server_item.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_server_migration.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_server_private.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_server_session.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_server_user.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_servermessage.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_serverresponse.h
# End Source File
# End Group
# Begin Group "transfer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\styxnet_transfer.cpp
# End Source File
# Begin Source File

SOURCE=.\styxnet_transfer.h
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ident.h
# End Source File
# Begin Source File

SOURCE=.\logging.cpp
# End Source File
# Begin Source File

SOURCE=.\logging.h
# End Source File
# Begin Source File

SOURCE=.\logging_dest.h
# End Source File
# Begin Source File

SOURCE=.\logging_destconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\logging_destconsole.h
# End Source File
# Begin Source File

SOURCE=.\logging_destdr2.cpp
# End Source File
# Begin Source File

SOURCE=.\logging_destdr2.h
# End Source File
# Begin Source File

SOURCE=.\stream.h
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32_critsec.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_critsec.h
# End Source File
# Begin Source File

SOURCE=.\win32_dns.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_dns.h
# End Source File
# Begin Source File

SOURCE=.\win32_event.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_event.h
# End Source File
# Begin Source File

SOURCE=.\win32_file.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_file.h
# End Source File
# Begin Source File

SOURCE=.\win32_guid.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_guid.h
# End Source File
# Begin Source File

SOURCE=.\win32_mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_mutex.h
# End Source File
# Begin Source File

SOURCE=.\win32_socket.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_socket.h
# End Source File
# Begin Source File

SOURCE=.\win32_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\win32_thread.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp
# ADD CPP /Yc"std.h"
# End Source File
# Begin Source File

SOURCE=.\styxnet.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_data.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_event.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_session.h
# End Source File
# Begin Source File

SOURCE=.\styxnet_std.h
# End Source File
# End Target
# End Project
