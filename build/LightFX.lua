project "LightFX"
	kind "ConsoleApp"

	defines { "EMBREE_STATIC_LIB", }
	includedirs {
		"../3rd/src/lodepng",
		"../3rd/src/dirent/include",
		"../3rd/src/rapidjson/include",
		"../3rd/src/socket.io-client-cpp/src",
		"../3rd/src/websocketpp-0.8.2",
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
	
	-- embree
	links { "embree", "embree_avx", "embree_avx2", "embree_sse42", "simd", "lexers", "tasking", "sys.lib", "math.lib" }
	-- tbb
	filter { "configurations:Debug"  }
		links { "tbb_debug", "tbbmalloc_debug", "tbbmalloc_proxy_debug" }
	filter { "configurations:Release"  }
		links { "tbb", "tbbmalloc", "tbbmalloc_proxy" }
	filter{}
