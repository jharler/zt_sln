@echo off
set random_val=%random%

set compile_flags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -DZT_DLL -FC -Z7 -LD -I"E:\Development\Projects\ZeroTolerance"
set link_flags=-incremental:no -opt:ref -PDB:Debug\%random_val%.hotload.dll.pdb -OUT:Debug\%random_val%.hotload.dll.dll user32.lib gdi32.lib winmm.lib shell32.lib

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
cl %compile_flags% ..\..\src\unity_dll.cpp -Fmdll.map /link %link_flags%


::REM 64-bit build
::del *.pdb > NUL 2> NUL
::cl %CommonCompilerFlags% ..\handmade\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
::cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
::popd
