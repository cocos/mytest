project "LightFX"
	kind "ConsoleApp"

	disablewarnings {
		"4100", -- unreferenced formal parameter
		"4201", -- use nameless struct/union
		"4267", -- convert size_t to int
		"4505", -- unreferenced local function
		"4996", -- deprecation function
	}

	defines { "EMBREE_STATIC_LIB", }
	sysincludedirs {
		"../3rd/src/rapidjson/include",
		"../3rd/src/websocketpp-0.8.2",
	}
	includedirs {
		"../3rd/src/lodepng",
		"../3rd/src/socket.io-client-cpp/src",
		"../source",
	}
	files {
		"../3rd/src/lodepng/lodepng.cpp",
		--"../3rd/src/socket.io-ext/internal/*.cpp",
		"../3rd/src/socket.io-client-cpp/src/sio_client.cpp",
		"../3rd/src/socket.io-client-cpp/src/sio_socket.cpp",
		"../3rd/src/socket.io-client-cpp/src/internal/*.cpp",
		"../source/**.*",
	}
	
	filter { "platforms:Win64" }
		includedirs {
			"../3rd/src/dirent/include",
		}
		-- embree
		links { "embree", "embree_avx", "embree_avx2", "embree_sse42", "simd", "lexers", "tasking", "sys.lib", "math.lib" }
	filter { "platforms:Win64", "configurations:Debug"  }
		-- tbb debug
		links { "tbb_debug", "tbbmalloc_debug", "tbbmalloc_proxy_debug" }
	filter { "platforms:Win64", "configurations:Release"  }
		-- tbb release
		links { "tbb", "tbbmalloc", "tbbmalloc_proxy" }
	filter{}
