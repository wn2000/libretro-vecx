cmake_minimum_required(VERSION 3.7)
project(vecx_libretro)
set (CMAKE_CXX_STANDARD 17)

set(ASAN off CACHE BOOL "Turn on Address Sanitizer")

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

if(ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/link.T)

target_link_libraries(${PROJECT_NAME}
    GLESv2
)

# Removes the 'lib' prefix on Linux
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
