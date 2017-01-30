@ECHO OFF

IF "%OS%" == "Windows_NT" GOTO WinNT

:Win9x

ECHO Windows9x

if NOT "%DR2DATA%" == "" GOTO CopySymDest

ECHO Copying Program Database %1.pdb to ..\..\data\
xcopy /y %1.pdb ..\..\data\

ECHO Copying Executable %1.exe to ..\..\data\
xcopy /y %1.exe ..\..\data\

GOTO :END

:CopySymDest
ECHO Copying Program Database %1.pdb to %DR2DATA%
xcopy /y %1.pdb %DR2DATA%

ECHO Copying Executable %1.exe to %DR2DATA%
xcopy /y %1.exe %DR2DATA%

GOTO :END

:WinNT

ECHO WindowsNT

if NOT "%DR2DATA%" == "" GOTO CopySymDestNT

ECHO Making ..\..\data directory
mkdir ..\..\data

ECHO Copying Program Database %1.pdb to ..\..\data
copy %1.pdb ..\..\data

ECHO Copying Executable %1.exe to ..\..\data
copy %1.exe ..\..\data

GOTO :END

:CopySymDestNT
ECHO Copying Program Database %1.pdb to %DR2DATA%
copy %1.pdb %DR2DATA%

ECHO Copying Executable %1.exe to %DR2DATA%
copy %1.exe %DR2DATA%

GOTO :END

: additional release mode processing
IF NOT "%5" == "[Release]" GOTO END

ECHO Performing Release Step
: ..\tools\pecomp ..\..\data\%1.exe

:END
