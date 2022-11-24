build\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree.zip 
build\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx.zip 
build\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx2.zip 
build\unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_sse42.zip 

echo make solution
cd build\win
premake5 --os=windows vs2019

echo compile solution
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" bin\LightFX.sln /Project LightFX /Build Release /Out build.log

cd ..\..\
echo compress to target
for /f "tokens=1,* delims= " %%a in ("%date:/=-%") do (
set datestr=%%a
)
set filename=build\lightmap-tools-win32-%datestr%.zip
build\zip -j %filename% build\win\bin\Release\LightFX.exe