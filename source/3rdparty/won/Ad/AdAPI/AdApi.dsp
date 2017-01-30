# Microsoft Developer Studio Project File - Name="AdApi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AdApi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AdApi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AdApi.mak" CFG="AdApi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AdApi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdApi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Titan/API/Ad/AdAPI", NQSBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AdApi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADAPI_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".." /I "Format/Object" /I "Format/Base" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADAPI_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../adsample/Release/AdApi.dll"

!ELSEIF  "$(CFG)" == "AdApi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADAPI_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".." /I "Format/Object" /I "Format/Base" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADAPI_EXPORTS" /D "LOGGING" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../adsample/Debug/AdApi.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AdApi - Win32 Release"
# Name "AdApi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AdApi.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\AdCache.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\AdDisplay.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\AdEngine.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\AdProtocol.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\RegKey.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\Threadbase.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AdApi.h
# End Source File
# Begin Source File

SOURCE=.\AdCache.h
# End Source File
# Begin Source File

SOURCE=.\AdDisplay.h
# End Source File
# Begin Source File

SOURCE=.\AdEngine.h
# End Source File
# Begin Source File

SOURCE=.\AdProtocol.h
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\NetCommInterface.h
# End Source File
# Begin Source File

SOURCE=.\RegKey.h
# End Source File
# Begin Source File

SOURCE=.\Threadbase.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# End Group
# Begin Group "ImageMagick"

# PROP Default_Filter ""
# Begin Group "Base Source"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\ImageMagick\Base\8bim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\animate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\annotate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\avs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\blob.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\bmp.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\cache.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\cache_io.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\cmyk.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\colors.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\compress.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\dcm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\decorate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\delegates.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\display.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\dps.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\draw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\effects.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\enhance.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\ept.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\error.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\fax.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\fits.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\fpx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\gems.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\gif.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\gradation.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\gray.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\hdf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\histogram.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\html.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\icc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\icon.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\image.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\iptc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\jbig.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\jpeg.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\label.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\logo.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\mac.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\magick.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\map.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\matte.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\memory.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\miff.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\monitor.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\mono.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\montage.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\mtv.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\nt.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\null.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pcd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pcl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pcx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pdf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pict.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pix.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\plasma.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\png.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\PreRvIcccm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\preview.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\ps.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\ps2.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\ps3.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\psd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\pwp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\quantize.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\rgb.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\rla.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\rle.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\sct.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\segment.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\sfw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\sgi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\shear.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\signature.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\stegano.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\Stub.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\sun.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\tga.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\tiff.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\tile.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\tim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\timer.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\transform.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\ttf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\txt.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\uil.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\utility.c
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\uyvy.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\vicar.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\vid.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\viff.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\vms.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\wbmp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\widget.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\x.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xpm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xwd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xwindows.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\yuv.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\zoom.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Base Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ImageMagick\Base\acconfig.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\animate.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\api.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\blob.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\cache.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\cache_io.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\classify.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\compress.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\defines.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\delegates.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\display.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\error.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\gems.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\image.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\mac.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\magick.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\memory.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\mng.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\monitor.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\nt.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\PreRvIcccm.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\quantize.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\timer.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\utility.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\version.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\vms.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\widget.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Base\xwindows.h
# End Source File
# End Group
# Begin Group "Object Source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\ImageMagick\Object\Blob2.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Color.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Drawable.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Exception.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Functions.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Geometry.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Image2.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\LastError.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Montage.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Options.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Pixels.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Thread.cpp
# ADD CPP /I "ImageMagick/Object" /I "ImageMagick/Base" /D "_LIB" /D "_VISUALC_"
# SUBTRACT CPP /I ".." /I "Format/Object" /I "Format/Base"
# End Source File
# End Group
# Begin Group "Object Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ImageMagick\Object\Blob.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Color.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Drawable.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Exception.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Functions.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Geometry.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Image.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Include.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\LastError.h
# End Source File
# Begin Source File

SOURCE=".\ImageMagick\Object\Magick++.h"
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Montage.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Options.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Pixels.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\STL.h
# End Source File
# Begin Source File

SOURCE=.\ImageMagick\Object\Thread.h
# End Source File
# End Group
# End Group
# End Target
# End Project
