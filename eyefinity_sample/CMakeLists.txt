cmake_minimum_required(VERSION 3.15)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(eyefinity_sample VERSION 6.2.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_compile_definitions(_UNICODE)
add_compile_definitions(UNICODE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/inc ${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Core ${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Optional )

set(AGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/EyefinitySample.cpp)
set(AGS_PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/inc/amd_ags.h)

file( GLOB AGS_DXUT_CORE
    "${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Core/*.cpp"
)

file( GLOB AGS_DXUT_OPTIONAL
    "${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Optional/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DXUT/Optional/*.cpp"
)

add_library(amd_ags SHARED IMPORTED GLOBAL)
set_property(TARGET amd_ags PROPERTY IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/lib/amd_ags_x64.lib)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../ags_lib/lib/amd_ags_x64.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


add_executable(${PROJECT_NAME} WIN32 ${AGS_SOURCES} ${AGS_PUBLIC_HEADER} ${AGS_DXUT_CORE} ${AGS_DXUT_OPTIONAL})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC amd_ags D3D11 d3dcompiler comctl32 Imm32 Version winmm Usp10 Shlwapi)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

source_group("Source"                           FILES ${AGS_SOURCES}) 
source_group("Inc"                              FILES ${AGS_PUBLIC_HEADER})
source_group("DXUT Core"                        FILES ${AGS_DXUT_CORE})
source_group("DXUT Optional"                    FILES ${AGS_DXUT_OPTIONAL})

