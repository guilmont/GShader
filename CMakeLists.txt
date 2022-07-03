cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin")

### VENDOR ##################################################################

set(GRENDER_IMPLOT OFF CACHE BOOL "Implot won't be used")
add_subdirectory("vendor/GRender/GRender" GRender)

add_library(Json INTERFACE)
target_include_directories(Json INTERFACE "vendor/nlohmann/")

### PRIVATE LIBS #############################################################
### Colors
add_library(Colors STATIC "src/colors.cpp")
target_include_directories(Colors PRIVATE "include")
target_link_libraries(Colors PRIVATE GRender)

### Uniforms
add_library(Uniforms STATIC "src/uniforms.cpp")
target_include_directories(Uniforms PRIVATE "include")
target_link_libraries(Uniforms PRIVATE GRender)

### Dynamic Shader
add_library(DynamicShader STATIC "src/dynamicShader.cpp")
target_include_directories(DynamicShader PRIVATE "include")
target_link_libraries(DynamicShader PRIVATE GRender)

### Configuration file
add_library(ConfigFile STATIC "src/configFile.cpp")
target_include_directories(ConfigFile PRIVATE "include")
target_link_libraries(ConfigFile PRIVATE GRender Json Colors Uniforms)


### GShader ###################################################################
project(GShader)

add_executable(GShader "src/gshader.cpp")
target_include_directories(GShader PRIVATE "include")
target_link_libraries(GShader PRIVATE GRender Colors Uniforms DynamicShader ConfigFile Json)


if (WIN32)
	set_target_properties(GShader PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif()

install(TARGETS "GShader" 
	CONFIGURATIONS Release
		DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)

install(DIRECTORY "examples" 
	CONFIGURATIONS Release	
		DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

install(FILES "bin/layout.ini" 
	CONFIGURATIONS Release	
		DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)