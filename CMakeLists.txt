cmake_minimum_required(VERSION 3.22)

project(ItemHighlighter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MOD_ID "item_highlighter" CACHE STRING "Levi mod ID")
set(MOD_NAME "Item Highlighter" CACHE STRING "Levi mod display name")
set(MOD_AUTHOR "xiomi" CACHE STRING "Levi mod author")
set(MOD_VERSION "1.0.0" CACHE STRING "Levi mod version")
set(MOD_LIBRARY_NAME "item_highlighter" CACHE STRING "Native library name")
set(MOD_MINECRAFT_VERSIONS "[\"1.26.*\"]" CACHE STRING "Minecraft versions")

include(FetchContent)

FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 0.2.2
    SOURCE_SUBDIR cmake/preloader-sdk
)
FetchContent_MakeAvailable(preloader_android)

FetchContent_GetProperties(preloader_android)
set(LEVI_PRELOADER_INCLUDE_DIR "${preloader_android_SOURCE_DIR}/include")

if(NOT ANDROID)
    add_library(item_highlighter_syntax OBJECT src/ItemHighlighter.cpp)
    target_include_directories(item_highlighter_syntax PRIVATE "${LEVI_PRELOADER_INCLUDE_DIR}" src)
    target_compile_definitions(item_highlighter_syntax PRIVATE ITEM_HIGHLIGHTER_SYNTAX_ONLY=1)
    return()
endif()

if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "This build targets the supplied AArch64 Minecraft binary. ABI must be arm64-v8a.")
endif()

if(NOT TARGET preloader)
    add_subdirectory("${preloader_android_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_preloader_android")
endif()

add_library(${MOD_LIBRARY_NAME} SHARED src/ItemHighlighter.cpp)

target_include_directories(${MOD_LIBRARY_NAME} PRIVATE
    "${LEVI_PRELOADER_INCLUDE_DIR}"
    src
)

target_compile_options(${MOD_LIBRARY_NAME} PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -ffunction-sections
    -fdata-sections
    -fexceptions
)

target_link_options(${MOD_LIBRARY_NAME} PRIVATE
    "-Wl,--gc-sections"
    "-Wl,-z,max-page-size=16384"
)

find_library(log-lib log REQUIRED)
target_link_libraries(${MOD_LIBRARY_NAME} PRIVATE
    preloader
    ${log-lib}
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    @ONLY
)

set(PACKAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")
set(PACKAGE_MOD_DIR "${PACKAGE_ROOT}/${MOD_ID}")
set(PACKAGE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${MOD_ID}-${MOD_VERSION}-${CMAKE_ANDROID_ARCH_ABI}.levipack")

add_custom_target(levi_package
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${PACKAGE_ROOT}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PACKAGE_MOD_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${MOD_LIBRARY_NAME}>" "${PACKAGE_MOD_DIR}/lib${MOD_LIBRARY_NAME}.so"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/manifest.json" "${PACKAGE_MOD_DIR}/manifest.json"
    COMMAND "${CMAKE_COMMAND}" -E chdir "${PACKAGE_ROOT}" "${CMAKE_COMMAND}" -E tar "cf" "${PACKAGE_FILE}" --format=zip "${MOD_ID}"
    DEPENDS ${MOD_LIBRARY_NAME}
    VERBATIM
)
