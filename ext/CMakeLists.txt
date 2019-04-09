
include(CheckCXXCompilerFlag)

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/embree/common")
  message(FATAL_ERROR "The LightFX dependencies are missing! "
    "You probably did not clone the project with --recursive. It is possible to recover "
    "by calling \"git submodule update --init --recursive\"")
endif()

# Set a default build configuration (Release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

# Enable folders for projects in Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH ON)

  # Disable OpenGL deprecation warnings
  add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)

  if (HAS_CPP14_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif (HAS_CPP11_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    message(FATAL_ERROR "Unsupported compiler -- LightFX requires C++11 support!")
  endif()

  # Prefer libc++ in conjunction with Clang
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" HAS_LIBCPP)
    if (HAS_LIBCPP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -D_LIBCPP_VERSION")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
      message(STATUS "LightFX: using libc++.")
    else()
      message(WARNING "libc++ is recommended in conjunction with clang. Please insteall the libc++ development headers, provided e.g. by the packages 'libc++-dev' and 'libc++abi-dev' on Debian/Ubuntu.")
    endif()
  endif()

  # Enable link time optimization and set the default symbol
  # visibility to hidden (very important to obtain small binaries)
  if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
    # Default symbol visibility
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  endif()

  # Disable specific GCC 7 warnings
  if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-misleading-indentation -Wformat-truncation=0 -Wno-int-in-bool-context -Wimplicit-fallthrough=0")
  endif()
endif()

# Sanitize build environment for static build with C++11
if (MSVC)
  # Disable annoying secure CRT warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS")

  # We'll select the TBB library ourselves
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__TBB_NO_IMPLICIT_LINKAGE")

  # Parallel build on MSVC (all targets)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  #if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
  #
  #  # Disable Eigen vectorization for Windows 32 bit builds (issues with unaligned access segfaults)
  #  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEIGEN_DONT_ALIGN")
  #endif()

  # Static build
  set(CompilerFlags
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
elseif(APPLE)
  # Try to auto-detect a suitable SDK
  execute_process(COMMAND bash -c "xcodebuild -version -sdk | grep MacOSX | grep Path | head -n 1 | cut -f 2 -d ' '" OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
  string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
  string(REGEX REPLACE "^.*X([0-9.]*).sdk$" "\\1" CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_SYSROOT}")
endif()

#############################

# Add Boost and Websocket++ (headers only)
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/boost)
add_subdirectory(websocketpp)

# Build Thread Building Blocks
set(TBB_BUILD_SHARED          OFF  CACHE BOOL " " FORCE)
set(TBB_BUILD_STATIC          ON   CACHE BOOL " " FORCE)
set(TBB_BUILD_TBBMALLOC       ON   CACHE BOOL " " FORCE)
set(TBB_BUILD_TBBMALLOC_PROXY OFF  CACHE BOOL " " FORCE)
set(TBB_BUILD_TESTS           OFF  CACHE BOOL " " FORCE)
add_subdirectory(tbb)
set_property(TARGET tbb_static tbbmalloc_static tbb_def_files PROPERTY FOLDER "dependencies")
set_property(TARGET tbb_static tbbmalloc_static tbb_def_files PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(TARGET tbb_static tbbmalloc_static tbb_def_files PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(TARGET tbb_static tbbmalloc_static tbb_def_files PROPERTY ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Build Embree
set(TBB_ROOT "")
set(TBB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tbb/include)
set(TBB_LIBRARY tbb_static)
set(TBB_LIBRARY_MALLOC tbbmalloc_static)
set(ENABLE_TUTORIALS OFF CACHE BOOL " " FORCE)
set(ENABLE_ISPC_SUPPORT OFF CACHE BOOL " " FORCE)
set(ENABLE_STATIC_LIB ON CACHE BOOL " " FORCE)
# set(RTCORE_TASKING_SYSTEM INTERNAL)
add_subdirectory(embree)
add_dependencies(embree tbb_static tbbmalloc_static)
set_property(TARGET embree PROPERTY FOLDER "dependencies")

# Expose inlcude directories
set(BOOST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/boost)
set(WEBSOCKETPP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/websocketpp)
set(TBB_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/tbb/include)
set(EMBREE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/embree/include)
set(LODEPNG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/lodepng)
if(WIN32)
  set(DIRENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dirent/include)
endif()

#############################

# Compile remainder of the codebase with compiler warnings turned on
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # Re-enable disabled warnings
  # if(CMAKE_CXX_FLAGS MATCHES "-Wno-[^ ]+")
  #   string(REGEX REPLACE "-Wno-[^ ]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  # endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-anonymous-struct -Wno-c99-extensions -Wno-nested-anon-types -Wno-deprecated-register")
  endif()
endif()

set(CompilerFlags
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELWITHDEBINFO COMPILE_DEFINITIONS U_CMAKE_BUILD_TYPE
      CMAKE_MACOSX_RPATH
      BOOST_INCLUDE_DIR WEBSOCKETPP_INCLUDE_DIR TBB_INCLUDE_DIR EMBREE_INCLUDE_DIR
      LODEPNG_INCLUDE_DIR DIRENT_INCLUDE_DIR
)
foreach(CompilerFlag ${CompilerFlags})
  set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()
