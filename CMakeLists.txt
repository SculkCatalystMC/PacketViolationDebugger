cmake_minimum_required(VERSION 3.24)
project(PacketViolationDebugger LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This project supports Windows only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "This project requires MSVC.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
add_compile_options(/utf-8 /EHsc /utf-8 /W4 /O2 /GR- /GL)
add_link_options(/LTCG)

add_compile_definitions(
    MAGIC_ENUM_RANGE_MIN=0
    MAGIC_ENUM_RANGE_MAX=512
    UNICODE
    NOMINMAX
)

set(PROJECT_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")
set(PROJECT_LIB_DIR "${CMAKE_BINARY_DIR}/lib")

include(FetchContent)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.7
)
FetchContent_Declare(
    detours
    GIT_REPOSITORY https://github.com/microsoft/Detours.git
    GIT_TAG main
)
FetchContent_MakeAvailable(detours magic_enum)

file(GLOB_RECURSE PVD_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_library(PacketViolationDebugger SHARED
    ${PVD_SOURCES}
    src/Version.rc
)

set_target_properties(PacketViolationDebugger PROPERTIES
    OUTPUT_NAME "PacketViolationDebugger"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BIN_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_LIB_DIR}"
)

foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" cfg_upper)
    set_target_properties(PacketViolationDebugger PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${PROJECT_BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${PROJECT_BIN_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${PROJECT_LIB_DIR}"
    )
endforeach()

target_include_directories(PacketViolationDebugger PRIVATE
    src
    ${magic_enum_SOURCE_DIR}/include
)

if(NOT TARGET detours AND NOT TARGET detours::detours)
    file(GLOB DETOURS_SOURCES CONFIGURE_DEPENDS "${detours_SOURCE_DIR}/src/*.cpp")
    list(FILTER DETOURS_SOURCES EXCLUDE REGEX ".*/uimports\\.cpp$")

    if(NOT DETOURS_SOURCES)
        message(FATAL_ERROR "Detours sources were fetched but no .cpp files were found in ${detours_SOURCE_DIR}/src")
    endif()

    add_library(detours STATIC ${DETOURS_SOURCES})
    set(DETOURS_COMPAT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/detours-compat-include")
    file(MAKE_DIRECTORY "${DETOURS_COMPAT_INCLUDE_DIR}/detours")
    configure_file(
        "${detours_SOURCE_DIR}/src/detours.h"
        "${DETOURS_COMPAT_INCLUDE_DIR}/detours/detours.h"
        COPYONLY
    )
    target_include_directories(detours PUBLIC
        "${detours_SOURCE_DIR}/src"
        "${DETOURS_COMPAT_INCLUDE_DIR}"
    )
endif()

target_include_directories(PacketViolationDebugger PRIVATE
    $<TARGET_PROPERTY:detours,INTERFACE_INCLUDE_DIRECTORIES>
)

if(TARGET detours::detours)
    target_link_libraries(PacketViolationDebugger PRIVATE detours::detours)
elseif(TARGET detours)
    target_link_libraries(PacketViolationDebugger PRIVATE detours)
else()
    message(FATAL_ERROR "Detours dependency fetched, but no known CMake target was found to link.")
endif()

target_link_libraries(PacketViolationDebugger PRIVATE magic_enum::magic_enum)
