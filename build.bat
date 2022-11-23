tools\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree.zip 
tools\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx.zip 
tools\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx2.zip 
tools\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_sse42.zip 

echo make solution
cd build\win
premake5 --file=premakex.lua --os=windows vs2019

echo compile solution
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" bin\LightFX.sln /Project LightFX /Build Release /Out build.log
cd ..\..\

echo compress to target
set datestr=%date:~0,4%%date:~5,2%%date:~8,2%
set filename=build\lightmap-tools-win32-%datestr%.zip
tools\zip -j %filename% build\win\bin\Release\LightFX.exe