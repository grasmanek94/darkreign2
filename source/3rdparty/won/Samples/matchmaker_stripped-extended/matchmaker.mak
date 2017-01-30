# Microsoft Developer Studio Generated NMAKE File, Based on matchmaker.dsp
!IF "$(CFG)" == ""
CFG=matchmaker - Win32 Debug
!MESSAGE No configuration specified. Defaulting to matchmaker - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "matchmaker - Win32 Release" && "$(CFG)" != "matchmaker - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "matchmaker.mak" CFG="matchmaker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "matchmaker - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "matchmaker - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "matchmaker - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\matchmaker.exe"

!ELSE 

ALL : "TitanAPI - Win32 Release" "$(OUTDIR)\matchmaker.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"TitanAPI - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\matchmaker.obj"
	-@erase "$(INTDIR)\matchmakerex.obj"
	-@erase "$(INTDIR)\Utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\matchmaker.exe"
	-@erase "$(OUTDIR)\matchmaker.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\.." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\matchmaker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\matchmaker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib advapi32.lib user32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\matchmaker.pdb" /map:"$(INTDIR)\matchmaker.map" /machine:I386 /out:"$(OUTDIR)\matchmaker.exe" 
LINK32_OBJS= \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\matchmaker.obj" \
	"$(INTDIR)\matchmakerex.obj" \
	"$(INTDIR)\Utils.obj" \
	"..\..\Release\TitanAPI.lib"

"$(OUTDIR)\matchmaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "matchmaker - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\matchmaker.exe"

!ELSE 

ALL : "TitanAPI - Win32 Debug" "$(OUTDIR)\matchmaker.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"TitanAPI - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\matchmaker.obj"
	-@erase "$(INTDIR)\matchmakerex.obj"
	-@erase "$(INTDIR)\Utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\matchmaker.exe"
	-@erase "$(OUTDIR)\matchmaker.ilk"
	-@erase "$(OUTDIR)\matchmaker.map"
	-@erase "$(OUTDIR)\matchmaker.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\.." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\matchmaker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\matchmaker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib advapi32.lib user32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\matchmaker.pdb" /map:"$(INTDIR)\matchmaker.map" /debug /machine:I386 /out:"$(OUTDIR)\matchmaker.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\matchmaker.obj" \
	"$(INTDIR)\matchmakerex.obj" \
	"$(INTDIR)\Utils.obj" \
	"..\..\Debug\TitanAPI.lib"

"$(OUTDIR)\matchmaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("matchmaker.dep")
!INCLUDE "matchmaker.dep"
!ELSE 
!MESSAGE Warning: cannot find "matchmaker.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "matchmaker - Win32 Release" || "$(CFG)" == "matchmaker - Win32 Debug"
SOURCE=.\game.cpp

"$(INTDIR)\game.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\matchmaker.cpp

"$(INTDIR)\matchmaker.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\matchmakerex.cpp

"$(INTDIR)\matchmakerex.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\common\Utils.cpp

"$(INTDIR)\Utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "matchmaker - Win32 Release"

"TitanAPI - Win32 Release" : 
   cd "\TITAN\API"
   $(MAKE) /$(MAKEFLAGS) /F .\TitanAPI.mak CFG="TitanAPI - Win32 Release" 
   cd ".\Samples\matchmaker stripped-extended"

"TitanAPI - Win32 ReleaseCLEAN" : 
   cd "\TITAN\API"
   $(MAKE) /$(MAKEFLAGS) /F .\TitanAPI.mak CFG="TitanAPI - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Samples\matchmaker stripped-extended"

!ELSEIF  "$(CFG)" == "matchmaker - Win32 Debug"

"TitanAPI - Win32 Debug" : 
   cd "\TITAN\API"
   $(MAKE) /$(MAKEFLAGS) /F .\TitanAPI.mak CFG="TitanAPI - Win32 Debug" 
   cd ".\Samples\matchmaker stripped-extended"

"TitanAPI - Win32 DebugCLEAN" : 
   cd "\TITAN\API"
   $(MAKE) /$(MAKEFLAGS) /F .\TitanAPI.mak CFG="TitanAPI - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Samples\matchmaker stripped-extended"

!ENDIF 


!ENDIF 

