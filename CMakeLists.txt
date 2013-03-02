PROJECT( vecx )

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

set (
	SOURCES
	e6809.c
	e6809.h
	e8910.c
	e8910.h
	vecx.c
	vecx.h
)

if (CMAKE_BUILD_TYPE STREQUAL "Libretro")
	message("--- Libretro Build ---")
	LIST (APPEND
		SOURCES
			osint_libretro.c
			libretro.h
			
	)	
	add_definitions(-DHAVE_LIBRETRO -g )
	add_library(retro-vecx SHARED ${SOURCES})
	install(TARGETS retro-vecx DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/libretro)

else (CMAKE_BUILD_TYPE STREQUAL "Libretro")
	message("--- SDL (Standalone) Build ---")
	FIND_PACKAGE( SDL REQUIRED )
	FIND_PACKAGE( SDL_gfx REQUIRED )
	FIND_PACKAGE( SDL_image REQUIRED )
	LIST (APPEND
		SOURCES
			osint.c
	)
	link_libraries( ${SDL} ${SDL_LIBRARY} ${SDLGFX_LIBRARY} ${SDLIMAGE_LIBRARY})
	include_directories( ${SDL_INCLUDE_DIR} ${SDLGFX_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
	add_executable( vecx ${SOURCES} )

endif (CMAKE_BUILD_TYPE STREQUAL "Libretro")
