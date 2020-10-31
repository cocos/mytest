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
  # LodePNG library
  ${LODEPNG_INCLUDE_DIR}
  # dirent library (on windows)
  ${DIRENT_INCLUDE_DIR}
  # socket.io
  ext/socket.io-client-cpp/src
  # rapidjson
  ext/rapidjson/include
  
)

add_executable(LightFX
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
  include/LFX_Log.h
  include/LFX_Stream.h
  include/LFX_Vec2.h
  include/LFX_World.h
  include/LFX_EmbreeScene.h
  include/LFX_ILBakerRandom.h
  include/LFX_Mat3.h
  include/LFX_Terrain.h
  include/LFX_Vec2.inl

  src/LFX_AOBaker.cpp
  src/LFX_Baker.cpp
  src/LFX_ILBakerRandom.cpp
  src/LFX_ImagePNG.cpp
  src/LFX_RasterizerSoft.cpp
  src/LFX_RasterizerZSpan.cpp
  src/LFX_Thread.cpp
  src/LFX_EmbreeScene.cpp
  src/LFX_ILBakerRaytrace.cpp
  src/LFX_Light.cpp
  src/LFX_Log.cpp
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
  src/main3.cpp
  ${LODEPNG_INCLUDE_DIR}/lodepng.cpp
  ext/socket.io-ext/internal/sio_client_impl.cpp
  ext/socket.io-ext/internal/sio_packet.cpp
  ext/socket.io-client-cpp/src/sio_client.cpp
  ext/socket.io-client-cpp/src/sio_socket.cpp
)

# this won't work out of the box because FindBoost need everything to be ready before it runs
# but when need to do regular rebuilds, this might come in handy
# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#   set(BOOST_ADDRESS_MODE 64)
# elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#   set(BOOST_ADDRESS_MODE 32)
# endif()
# add_custom_target(
#   build_boost
#   COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost
#   # COMMAND call bootstrap
#   # COMMAND b2 headers
#   COMMAND b2 address-model=${BOOST_ADDRESS_MODE} link=static runtime-link=static stage --with-system --with-date_time --with-regex
# )
# add_dependencies(LightFX build_boost)

set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS system date_time regex random REQUIRED)

add_definitions(-DEMBREE_STATIC_LIB) # dirty fix for win32 embree static build bug
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
