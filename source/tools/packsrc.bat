@ECHO OFF

: Rar up the source, cod files and the map file
ECHO Packing up Source, COD and MAP files
RAR a source.rar ..\*.cod ..\*.map ..\*.cpp ..\*.h ..\*.asm ..\*.tsm ..\*.cc -r -m5 -md1024 -av

:END