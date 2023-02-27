project(new_cmd)
cmake_minimum_required(VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR} extern)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-pipe)
endif()

add_subdirectory(extern/patterns)
add_subdirectory(extern/magic_enum)

# --- new_cmd
FILE(GLOB cmd_src CONFIGURE_DEPENDS "src/*.c*")
FILE(GLOB cmd_hdr CONFIGURE_DEPENDS "src/*.h*")

add_executable(new_cmd
    extern/lz77.hh extern/sinfl.hh
    ${cmd_src} ${cmd_hdr}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(new_cmd PRIVATE
		$<$<COMPILE_LANGUAGE:CXX>:-fpermissive -w>
	)
endif()

target_link_libraries(new_cmd mpark_patterns magic_enum)


# --- compressor
add_executable(compressor
    src/sdk/compress.c src/utility.c
    src/dllheader.hh
    src/sdk/lz77comp.h src/sdk/sdefl.h
)
