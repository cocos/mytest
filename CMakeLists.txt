cmake_minimum_required (VERSION 2.8.10)
project(LightFX)

add_subdirectory(ext ext_build)

include_directories(
  # LightFX include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # Boost library
  ${BOOST_INCLUDE_DIR}
  # websocket++ library
  ${WEBSOCKETPP_INCLUDE_DIR}
  # TBB library
  ${TBB_INCLUDE_DIR}
  # Embree library
  ${EMBREE_INCLUDE_DIR}
)

add_executable(LightFX
  include/dirent.h
  include/LFX_Entity.h
  include/LFX_ILBakerRaytrace.h
  include/LFX_Math.h
  include/LFX_Texture.h
  include/LFX_Vec3.h
  include/LFX_Aabb.h
  include/LFX_File.h
  include/LFX_ILBakerSamples.h
  include/LFX_Mesh.h
  include/LFX_TextureAtlas.h
  include/LFX_Vec3.inl
  include/LFX_Baker.h
  include/LFX_AOBaker.h
  include/LFX_Geom.h
  include/LFX_ILBakerSampling.h
  include/LFX_Rasterizer.h
  include/LFX_Thread.h
  include/LFX_Vec4.h
  include/LFX_BSP.h
  include/LFX_ILBaker.h
  include/LFX_Image.h
  include/LFX_RasterizerSoft.h
  include/LFX_Types.h
  include/LFX_Vec4.inl
  include/LFX_Color.h
  include/LFX_ILBakerMath.h
  include/LFX_Light.h
  include/LFX_Stream.h
  include/LFX_Vec2.h
  include/LFX_World.h
  include/LFX_EmbreeScene.h
  include/LFX_ILBakerRandom.h
  include/LFX_Mat3.h
  include/LFX_Terrain.h
  include/LFX_Vec2.inl
  include/lodepng.h

  src/LFX_AOBaker.cpp
  src/LFX_Baker.cpp
  src/LFX_ILBakerRandom.cpp
  src/LFX_ImagePNG.cpp
  src/LFX_RasterizerSoft.cpp
  src/LFX_Thread.cpp
  src/LFX_EmbreeScene.cpp
  src/LFX_ILBakerRaytrace.cpp
  src/LFX_Light.cpp
  src/LFX_Stream.cpp
  src/LFX_Types.cpp
  src/LFX_Entity.cpp
  src/LFX_ILBakerSamples.cpp
  src/LFX_Mesh.cpp
  src/LFX_Terrain.cpp
  src/LFX_World.cpp
  src/LFX_File.cpp
  src/LFX_ImageBMP.cpp
  src/LFX_Rasterizer.cpp
  src/LFX_TextureAtlas.cpp
  src/main2.cpp
  src/lodepng.cpp
)

# this won't work because FindBoost need everything to be ready before it runs
# besides, generally we only need to build them once
# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#   set(BOOST_ADDRESS_MODE 64)
# elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#   set(BOOST_ADDRESS_MODE 32)
# endif()
# add_custom_target(
#   boost_bootstrap
#   COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost
#   COMMAND call bootstrap
#   COMMAND b2 headers
#   COMMAND b2 address-model=${BOOST_ADDRESS_MODE} runtime-link=static stage --with-system --with-date_time --with-regex
# )
# add_dependencies(LightFX boost_bootstrap boost_headers boost_build)

set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS system date_time regex REQUIRED)

target_link_libraries(LightFX ${Boost_LIBRARIES} embree)

# Force colored output for the ninja generator
if (CMAKE_GENERATOR STREQUAL "Ninja")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

# vim: set et ts=2 sw=2 ft=cmake nospell:
