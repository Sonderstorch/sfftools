@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
nmake nodebug=1 /f makefile.vc %1
