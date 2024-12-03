solution "LightFX"
    configurations { "Debug", "Release" }
	language "C++"
    characterset "MBCS"
	location "bin"

	if _OPTIONS["build"] then
		configurations { _OPTIONS["build"] }
	end

	--windows configure
	filter {"system:windows"}
		platforms { "Win64" }
		system "Windows"
		architecture "x64"
   
	filter { "system:windows", "configurations:Debug" }
		symbols "On"
		targetdir "bin/Debug"
		debugdir "bin/Debug"
		defines { "DEBUG", "_DEBUG" }
		includedirs {
			"../3rd/x64-win/include",
			"./vcpkg/installed/x64-windows-static/include",
		}
		libdirs {
			"../3rd/x64-win/debug/lib",
			"../3rd/embree/debug/lib",
			"./vcpkg/installed/x64-windows-static/lib",
		}
		
	filter { "system:windows", "configurations:Release" }
		optimize "On"
		debugdir "bin/Release"
		targetdir "bin/Release"
		defines { "NDEBUG" }
		includedirs {
			"../3rd/x64-win/include",
			"./vcpkg/installed/x64-windows-static/include",
		}
		libdirs {
			"../3rd/x64-win/lib",
			"../3rd/embree/lib",
			"./vcpkg/installed/x64-windows-static/lib",
		}
		
    -- macOS configure
    filter {"system:macosx"}
		platforms { "Mac" }
		system "macosx"
		flags { "C++11" }
		architecture "x64"

	filter {"system:macosx"}
		platforms { "Mac" }
		system "macosx"
		flags { "C++11" }
		architecture "arm64"
		
   
	filter { "system:macosx", "configurations:Debug" }
		symbols "On"
		targetdir "bin/Debug"
		debugdir "bin/Debug"
		defines { "DEBUG", "_DEBUG" }
		externalincludedirs {
			"./vcpkg/installed/uni-osx/include",
		}
		libdirs {
			"./vcpkg/installed/uni-osx/lib",
		}
		
	filter { "system:macosx", "configurations:Release" }
		optimize "On"
		debugdir "bin/Release"
		targetdir "bin/Release"
		defines { "NDEBUG" }
		externalincludedirs {
			"./vcpkg/installed/uni-osx/include",
		}
		libdirs {
			"./vcpkg/installed/uni-osx/lib",
		}
		
	filter {}

require "./LightFX"

filter {"system:macosx"}
    postbuildcommands {
        "mkdir -p bin/Release/uni",
        "lipo -create bin/Release/x64/LightFx bin/Release/arm64/LightFx -output bin/Release/uni/LightFx"
    }