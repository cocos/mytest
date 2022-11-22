expand 3rd\embree\lib\embree.zip 3rd\embree\lib\embree.lib
expand 3rd\embree\lib\embree_avx.zip 3rd\embree\lib\embree_avx.lib
expand 3rd\embree\lib\embree_avx2.zip 3rd\embree\lib\embree_avx2.lib
expand 3rd\embree\lib\embree_sse42.zip 3rd\embree\lib\embree_sse42.lib

echo make solution
cd build\win
premake5 --file=premakex.lua --os=windows vs2019
echo compile solution
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" bin/LightFX.sln /Rebuild Release