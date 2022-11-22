unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree.zip 
unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx.zip 
unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_avx2.zip 
unzip -n -d 3rd\embree\lib 3rd\embree\lib\embree_sse42.zip 

echo make solution
cd build\win
premake5 --file=premakex.lua --os=windows vs2019
echo compile solution
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" bin\LightFX.sln /Project LightFX /Build Release /Out build.log