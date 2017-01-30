# Microsoft Developer Studio Project File - Name="Base" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Base - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Base.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Base.mak" CFG="Base - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Base - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Base - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Base - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../Compress" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_VISUALC_" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Base - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../Compress" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_VISUALC_" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Base - Win32 Release"
# Name "Base - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\8bim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\animate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\annotate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\avs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\blob.c
# End Source File
# Begin Source File

SOURCE=.\bmp.c
# End Source File
# Begin Source File

SOURCE=.\cache.c
# End Source File
# Begin Source File

SOURCE=.\cache_io.c
# End Source File
# Begin Source File

SOURCE=.\cmyk.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\colors.c
# End Source File
# Begin Source File

SOURCE=.\compress.c
# End Source File
# Begin Source File

SOURCE=.\dcm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\decorate.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\delegates.c
# End Source File
# Begin Source File

SOURCE=.\display.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dps.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\draw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\effects.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\enhance.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ept.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\error.c
# End Source File
# Begin Source File

SOURCE=.\fax.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\fits.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\fpx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gems.c
# End Source File
# Begin Source File

SOURCE=.\gif.c
# End Source File
# Begin Source File

SOURCE=.\gradation.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gray.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\hdf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\histogram.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\html.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\icc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\icon.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\iptc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\jbig.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\jpeg.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\label.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\logo.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mac.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\magick.c
# End Source File
# Begin Source File

SOURCE=.\map.c
# End Source File
# Begin Source File

SOURCE=.\matte.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\miff.c
# End Source File
# Begin Source File

SOURCE=.\monitor.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mono.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\montage.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mtv.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\nt.c
# End Source File
# Begin Source File

SOURCE=.\null.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pcd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pcl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pcx.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pdf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pict.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pix.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\plasma.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\png.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pnm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\PreRvIcccm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\preview.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ps.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ps2.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ps3.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\psd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pwp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\quantize.c

!IF  "$(CFG)" == "Base - Win32 Release"

!ELSEIF  "$(CFG)" == "Base - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rgb.c
# End Source File
# Begin Source File

SOURCE=.\rla.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rle.c
# End Source File
# Begin Source File

SOURCE=.\sct.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\segment.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sfw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\sgi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\shear.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\signature.c
# End Source File
# Begin Source File

SOURCE=.\stegano.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Stub.c

!IF  "$(CFG)" == "Base - Win32 Release"

!ELSEIF  "$(CFG)" == "Base - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sun.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tga.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tiff.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tile.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\tim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\transform.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ttf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\txt.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\uil.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\utility.c
# End Source File
# Begin Source File

SOURCE=.\uyvy.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\vicar.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\vid.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\viff.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\vms.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\wbmp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\widget.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\x.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xbm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xpm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xwd.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\xwindows.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\yuv.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zoom.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\acconfig.h
# End Source File
# Begin Source File

SOURCE=.\animate.h
# End Source File
# Begin Source File

SOURCE=.\api.h
# End Source File
# Begin Source File

SOURCE=.\blob.h
# End Source File
# Begin Source File

SOURCE=.\cache.h
# End Source File
# Begin Source File

SOURCE=.\cache_io.h
# End Source File
# Begin Source File

SOURCE=.\classify.h
# End Source File
# Begin Source File

SOURCE=.\compress.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\delegates.h
# End Source File
# Begin Source File

SOURCE=.\display.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\gems.h
# End Source File
# Begin Source File

SOURCE=.\image.h
# End Source File
# Begin Source File

SOURCE=.\mac.h
# End Source File
# Begin Source File

SOURCE=.\magick.h
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\mng.h
# End Source File
# Begin Source File

SOURCE=.\monitor.h
# End Source File
# Begin Source File

SOURCE=.\nt.h
# End Source File
# Begin Source File

SOURCE=.\PreRvIcccm.h
# End Source File
# Begin Source File

SOURCE=.\quantize.h
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\utility.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\vms.h
# End Source File
# Begin Source File

SOURCE=.\widget.h
# End Source File
# Begin Source File

SOURCE=.\xwindows.h
# End Source File
# End Group
# End Target
# End Project
