cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin")

set(GRENDER_IMPLOT OFF CACHE BOOL "Implot won't be used")
set(ASSETS_DIR "assets")
add_subdirectory("vendor/GRender/GRender" GRender)

project(GShader)

add_library(Colors STATIC "colors.h" "colors.cpp")
target_link_libraries(Colors PRIVATE imgui glm)

add_executable(GShader
	"gshader.h"       "gshader.cpp"
	"dynamicShader.h" "dynamicShader.cpp"
)

target_link_libraries(GShader PRIVATE GRender Colors)
target_compile_definitions(GShader PRIVATE ASSETS="../${ASSETS_DIR}")

if (WIN32)
	set_target_properties(GShader PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif()