set MSC_VER=vs2019
if "%1" == "vs2017" (
	set MSC_VER=vs2017
)

premake5 --os=windows %MSC_VER%