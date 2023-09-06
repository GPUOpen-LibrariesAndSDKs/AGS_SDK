cmake_minimum_required(VERSION 3.15)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

if(AGS_INTERNAL_DEBUG)
project(ExtensionsSample12_internal VERSION 6.2.0)
else()
project(ExtensionsSample12 VERSION 6.2.0)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/inc)

file( GLOB EXTENSIONSSAMPLE12_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file( GLOB EXTENSIONSSAMPLE12_HLSL
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hlsl"
)
set(AGS_PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/inc/amd_ags.h)

source_group("Source"                           FILES ${EXTENSIONSSAMPLE12_SOURCES}) 
source_group("Inc"                              FILES ${AGS_PUBLIC_HEADER})
source_group("Shaders"                          FILES ${EXTENSIONSSAMPLE12_HLSL})

set_source_files_properties(${EXTENSIONSSAMPLE12_HLSL} PROPERTIES VS_TOOL_OVERRIDE "Text")


if(AGS_INTERNAL_DEBUG)
add_subdirectory(../ags_lib ags_internal)
add_executable(${PROJECT_NAME} WIN32 ${EXTENSIONSSAMPLE12_SOURCES} ${EXTENSIONSSAMPLE12_HLSL} ${AGS_PUBLIC_HEADER})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/lib/amd_ags_x64_2022_MDd.lib d3d12 dxgi d3dcompiler dxguid)

else()
add_library(amd_ags SHARED IMPORTED GLOBAL)
set_property(TARGET amd_ags PROPERTY IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/lib/amd_ags_x64.lib)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/lib/amd_ags_x64.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(${PROJECT_NAME} WIN32 ${EXTENSIONSSAMPLE12_SOURCES} ${EXTENSIONSSAMPLE12_HLSL} ${AGS_PUBLIC_HEADER})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC amd_ags d3d12 dxgi d3dcompiler dxguid)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

set_property(DIRECTORY ${CMAKE_PROJECT_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
