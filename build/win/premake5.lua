solution "LightFX"
	platforms { "Win64" }
	configurations { "Debug", "Release" }
	system "Windows"
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
			"../../3rd/x64-win/include",
		}
		libdirs {
			"../../3rd/x64-win/debug/lib",
		}
		
	filter { "configurations:Release" }
		optimize "On"
		debugdir "bin/Release"
		targetdir "bin/Release"
		defines { "NDEBUG" }
		includedirs {
			"../../3rd/x64-win/include",
		}
		libdirs {
			"../../3rd/x64-win/lib",
		}
		
	filter {}

require "../LightFX"