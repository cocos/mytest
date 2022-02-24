solution "LightFX"
	platforms { "Mac" }
	configurations { "Debug", "Release" }
	system "macosx"
	language "C++"
	architecture "x64"
	characterset "MBCS"
	location "bin"
   
	filter { "configurations:Debug" }
		symbols "On"
		targetdir "bin/Debug"
		debugdir "bin/Debug"
		defines { "DEBUG", "_DEBUG" }
		includedirs {
			"../../3rd/x64-osx/include",
		}
		libdirs {
			"../../3rd/x64-osx/debug/lib",
		}
		
	filter { "configurations:Release" }
		optimize "On"
		debugdir "bin/Release"
		targetdir "bin/Release"
		defines { "NDEBUG" }
		includedirs {
			"../../3rd/x64-osx/include",
		}
		libdirs {
			"../../3rd/x64-osx/lib",
		}
		
	filter {}

require "../LightFX"