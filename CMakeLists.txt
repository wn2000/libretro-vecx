cmake_minimum_required(VERSION 3.7)
project(vecx_libretro)
set (CMAKE_CXX_STANDARD 17)

include_directories(
    libretro-common/include
)

add_definitions(
    -D__LIBRETRO__ -DHAVE_STRINGS_H -DHAVE_STDINT_H -DHAVE_INTTYPES_H
    -DHAS_GPU
    -DHAVE_OPENGLES -DHAVE_OPENGLES2
)
add_link_options(-Wl,-no-undefined -static-libgcc -static-libstdc++)

add_library(${PROJECT_NAME} SHARED
    vecx.cpp
    libretro.cpp
    e6809.c
    e8910.c
    libretro-common/glsym/rglgen.c
    libretro-common/glsym/glsym_es2.c
    )
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/link.T)

target_link_libraries(${PROJECT_NAME}
    GLESv2
)

# Removes the 'lib' prefix on Linux
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
