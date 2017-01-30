# Microsoft Developer Studio Project File - Name="graphics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=graphics - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "graphics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "graphics.mak" CFG="graphics - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "graphics - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "graphics - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "graphics - Win32 Development" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/graphics", YIAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "graphics - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/graphics"
# PROP Intermediate_Dir "../Release/graphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /FAcs /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "graphics - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/graphics"
# PROP Intermediate_Dir "../Debug/graphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /Od /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "DEVELOPMENT" /Yu"std.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "graphics - Win32 Development"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "graphics___Win32_Development"
# PROP BASE Intermediate_Dir "graphics___Win32_Development"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Development/graphics"
# PROP Intermediate_Dir "../Development/graphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAcs /Yu"std.h" /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /Zi /O2 /Oy- /I "../3rdparty" /I "../coregame" /I "../coregame_ai" /I "../coregame_interface" /I "../coregame_objects" /I "../coregame_orders" /I "../coregame_particles" /I "../coregame_tasks" /I "../game" /I "../graphics" /I "../interface" /I "../main" /I "../multiplayer" /I "../sound" /I "../system" /I "../util" /FI"std.h" /D "WIN32" /D "NDEBUG" /D "DEVELOPMENT" /FAcs /Yu"std.h" /FD /c
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

# Name "graphics - Win32 Release"
# Name "graphics - Win32 Debug"
# Name "graphics - Win32 Development"
# Begin Group "bucket"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bucket.cpp
# End Source File
# Begin Source File

SOURCE=.\bucket.h
# End Source File
# Begin Source File

SOURCE=.\bucket_inline.h
# End Source File
# Begin Source File

SOURCE=.\BucketDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\BucketDesc.h
# End Source File
# Begin Source File

SOURCE=.\buckets_light.cpp
# End Source File
# Begin Source File

SOURCE=.\buckets_lock.cpp
# End Source File
# Begin Source File

SOURCE=.\Primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\Primitive.h
# End Source File
# Begin Source File

SOURCE=.\TranBucket.cpp
# End Source File
# Begin Source File

SOURCE=.\TranBucket.h
# End Source File
# End Group
# Begin Group "camera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\camera_test.cpp
# End Source File
# End Group
# Begin Group "light"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\light.cpp
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\lightrend.cpp
# End Source File
# Begin Source File

SOURCE=.\lightvertscamera.cpp
# End Source File
# Begin Source File

SOURCE=.\lightvertsmodel.cpp
# End Source File
# End Group
# Begin Group "material"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\bitmap.h
# End Source File
# Begin Source File

SOURCE=.\bitmap_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\bitmapclip.h
# End Source File
# Begin Source File

SOURCE=.\bitmapdec.h
# End Source File
# Begin Source File

SOURCE=.\bitmapprimitive.cpp
# End Source File
# Begin Source File

SOURCE=.\bitmapprimitive.h
# End Source File
# Begin Source File

SOURCE=.\material.cpp
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# End Group
# Begin Group "mesh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\anim.cpp
# End Source File
# Begin Source File

SOURCE=.\anim.h
# End Source File
# Begin Source File

SOURCE=.\family.cpp
# End Source File
# Begin Source File

SOURCE=.\family.h
# End Source File
# Begin Source File

SOURCE=.\godfile.cpp
# End Source File
# Begin Source File

SOURCE=.\godfile.h
# End Source File
# Begin Source File

SOURCE=.\lex.h
# End Source File
# Begin Source File

SOURCE=.\mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\mesh.h
# End Source File
# Begin Source File

SOURCE=.\mesh_render_vertex.cpp
# End Source File
# Begin Source File

SOURCE=.\mesh_render_vertex_1.cpp
# End Source File
# Begin Source File

SOURCE=.\mesh_render_vertextl.cpp
# End Source File
# Begin Source File

SOURCE=.\meshconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\meshconfig.h
# End Source File
# Begin Source File

SOURCE=.\meshent.cpp
# End Source File
# Begin Source File

SOURCE=.\meshent.h
# End Source File
# Begin Source File

SOURCE=.\meshent_render_vertex.cpp
# End Source File
# Begin Source File

SOURCE=.\meshent_render_vertextl.cpp
# End Source File
# Begin Source File

SOURCE=.\meshent_select.cpp
# End Source File
# Begin Source File

SOURCE=.\meshload.cpp
# End Source File
# Begin Source File

SOURCE=.\meshman.cpp
# End Source File
# Begin Source File

SOURCE=.\meshmrm.h
# End Source File
# Begin Source File

SOURCE=.\meshoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\meshoptions.h
# End Source File
# Begin Source File

SOURCE=.\meshread.cpp
# End Source File
# Begin Source File

SOURCE=.\meshview.cpp
# End Source File
# Begin Source File

SOURCE=.\meshview.h
# End Source File
# Begin Source File

SOURCE=.\meshwrite.cpp
# End Source File
# End Group
# Begin Group "mesheffects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\effects_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\effects_utils.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_base_color.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_base_color.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_color.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_color.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_fade.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_fade.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_glow.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_glow.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_liquidmetal.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_liquidmetal.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_plane.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_plane.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_planebuild.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_planebuild.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_resource.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_scale.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_scale.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_system.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_system.h
# End Source File
# Begin Source File

SOURCE=.\mesheffect_textcrossfade.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffect_textcrossfade.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_color.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_color.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_fade.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_fade.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_glow.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_glow.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_resource.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_scale.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_scale.h
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_textcrossfade.cpp
# End Source File
# Begin Source File

SOURCE=.\mesheffecttype_textcrossfade.h
# End Source File
# End Group
# Begin Group "mrm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IMESH.h
# End Source File
# Begin Source File

SOURCE=.\IMESHUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\IMESHUtil.h
# End Source File
# Begin Source File

SOURCE=.\MRMGen.h
# End Source File
# Begin Source File

SOURCE=.\MRMGenParams.h
# End Source File
# Begin Source File

SOURCE=.\MRMUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\MRMUpdate.h
# End Source File
# End Group
# Begin Group "terrain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\heightfield.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain.h
# End Source File
# Begin Source File

SOURCE=.\terrain_lod.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_priv.h
# End Source File
# Begin Source File

SOURCE=.\terrain_render_isometric.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_render_quick.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_render_rect.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_render_vertexc.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_render_vertextl.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_sky.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain_sprite.cpp
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gamemath.cpp
# End Source File
# Begin Source File

SOURCE=.\gamemath.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\statistics.h
# End Source File
# Begin Source File

SOURCE=.\Vertex.h
# End Source File
# End Group
# Begin Group "vid"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vid.cpp
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\vid_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_cmd.h
# End Source File
# Begin Source File

SOURCE=.\vid_cmd_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_cmd_mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_cmd_terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_cmd_var.h
# End Source File
# Begin Source File

SOURCE=.\vid_config.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_decl.h
# End Source File
# Begin Source File

SOURCE=.\vid_enumdx.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_heap.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_heap.h
# End Source File
# Begin Source File

SOURCE=.\vid_math.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_math.h
# End Source File
# Begin Source File

SOURCE=.\vid_private.h
# End Source File
# Begin Source File

SOURCE=.\vid_public.h
# End Source File
# Begin Source File

SOURCE=.\vid_settings.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\vidclip.cpp
# End Source File
# Begin Source File

SOURCE=.\vidclip.h
# End Source File
# Begin Source File

SOURCE=.\vidclip_line.cpp
# End Source File
# Begin Source File

SOURCE=.\vidclip_priv.h
# End Source File
# Begin Source File

SOURCE=.\vidclip_screen.cpp
# End Source File
# Begin Source File

SOURCE=.\vidclip_xtra.cpp
# End Source File
# Begin Source File

SOURCE=.\viderror.cpp
# End Source File
# Begin Source File

SOURCE=.\vidgraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\vidmirror.cpp
# End Source File
# Begin Source File

SOURCE=.\vidoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\vidrend.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\system\pchbuilder.cpp

!IF  "$(CFG)" == "graphics - Win32 Release"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "graphics - Win32 Debug"

# ADD CPP /Yc"std.h"

!ELSEIF  "$(CFG)" == "graphics - Win32 Development"

# ADD BASE CPP /Yc
# ADD CPP /Yc

!ENDIF 

# End Source File
# End Target
# End Project
