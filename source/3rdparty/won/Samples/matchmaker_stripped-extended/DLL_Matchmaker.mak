# Microsoft Developer Studio Generated NMAKE File, Based on DLL Matchmaker.dsp
!IF "$(CFG)" == ""
CFG=DLL Matchmaker - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DLL Matchmaker - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DLL Matchmaker - Win32 Release" && "$(CFG)" != "DLL Matchmaker - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DLL Matchmaker.mak" CFG="DLL Matchmaker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DLL Matchmaker - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DLL Matchmaker - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "DLL Matchmaker - Win32 Release"

OUTDIR=.\Release_DLL
INTDIR=.\Release_DLL
# Begin Custom Macros
OutDir=.\Release_DLL
# End Custom Macros

ALL : "$(OUTDIR)\DLL Matchmaker.exe"


CLEAN :
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\matchmaker.obj"
	-@erase "$(INTDIR)\Utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DLL Matchmaker.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DLLSAMPLE" /Fp"$(INTDIR)\DLL Matchmaker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DLL Matchmaker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib advapi32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\DLL Matchmaker.pdb" /machine:I386 /out:"$(OUTDIR)\DLL Matchmaker.exe" 
LINK32_OBJS= \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\matchmaker.obj" \
	"$(INTDIR)\Utils.obj"

"$(OUTDIR)\DLL Matchmaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DLL Matchmaker - Win32 Debug"

OUTDIR=.\Debug_DLL
INTDIR=.\Debug_DLL
# Begin Custom Macros
OutDir=.\Debug_DLL
# End Custom Macros

ALL : "$(OUTDIR)\DLL Matchmaker.exe"


CLEAN :
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\matchmaker.obj"
	-@erase "$(INTDIR)\Utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DLL Matchmaker.exe"
	-@erase "$(OUTDIR)\DLL Matchmaker.ilk"
	-@erase "$(OUTDIR)\DLL Matchmaker.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DLLSAMPLE" /Fp"$(INTDIR)\DLL Matchmaker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DLL Matchmaker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wondll.lib wsock32.lib advapi32.lib user32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\DLL Matchmaker.pdb" /debug /machine:I386 /out:"$(OUTDIR)\DLL Matchmaker.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\matchmaker.obj" \
	"$(INTDIR)\Utils.obj"

"$(OUTDIR)\DLL Matchmaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("DLL Matchmaker.dep")
!INCLUDE "DLL Matchmaker.dep"
!ELSE 
!MESSAGE Warning: cannot find "DLL Matchmaker.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DLL Matchmaker - Win32 Release" || "$(CFG)" == "DLL Matchmaker - Win32 Debug"
SOURCE=.\game.cpp

"$(INTDIR)\game.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\matchmaker.cpp

"$(INTDIR)\matchmaker.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\common\Utils.cpp

"$(INTDIR)\Utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

