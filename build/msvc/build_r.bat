@echo off
set random_val=%random%

set compile_flags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -DZT_DLL -FC -Z7 -LD -I"E:\Development\Projects\ZeroTolerance"
set link_flags=-incremental:no -opt:ref -PDB:Release\%random_val%.reload.dll.pdb -OUT:Release\%random_val%.reload.dll.dll user32.lib gdi32.lib winmm.lib shell32.lib


call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
cl %compile_flags% ..\..\src\unity_dll.cpp -Fmdll.map /link %link_flags%


::REM 64-bit build
::del *.pdb > NUL 2> NUL
::cl %CommonCompilerFlags% ..\handmade\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
::cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
::popd



::R
::/GS /GL /analyze- /W3 /Gy /Zc:wchar_t /I"E:\Development\Projects\ZeroTolerance" /Zi /Gm- /O2 /Fd"Release\vc120.pdb" /fp:precise /D "ZT_DLL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DLL_EXPORTS" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /Fa"Release\" /nologo /Fo"Release\" /Fp"Release\dll.pch" 
::/OUT:"E:\Development\Projects\zt_sln\build\msvc\Release\dll.dll" /MANIFEST /LTCG /NXCOMPAT /PDB:"E:\Development\Projects\zt_sln\build\msvc\Release\dll.pdb" /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /IMPLIB:"E:\Development\Projects\zt_sln\build\msvc\Release\dll.lib" /DEBUG /DLL /MACHINE:X86 /OPT:REF /SAFESEH /INCREMENTAL:NO /PGD:"E:\Development\Projects\zt_sln\build\msvc\Release\dll.pgd" /SUBSYSTEM:WINDOWS /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"Release\dll.dll.intermediate.manifest" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 

::D
::/GS /analyze- /W3 /Zc:wchar_t /I"E:\Development\Projects\ZeroTolerance" /ZI /Gm /Od /Fd"Debug\vc120.pdb" /fp:precise /D "ZT_DLL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DLL_EXPORTS" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /Oy- /MDd /Fa"Debug\" /nologo /Fo"Debug\" /Fp"Debug\dll.pch" 
::/OUT:"E:\Development\Projects\zt_sln\build\msvc\Debug\dll.dll" /MANIFEST /NXCOMPAT /PDB:"E:\Development\Projects\zt_sln\build\msvc\Debug\dll.pdb" /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /IMPLIB:"E:\Development\Projects\zt_sln\build\msvc\Debug\dll.lib" /DEBUG /DLL /MACHINE:X86 /INCREMENTAL /PGD:"E:\Development\Projects\zt_sln\build\msvc\Debug\dll.pgd" /SUBSYSTEM:WINDOWS /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"Debug\dll.dll.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 

