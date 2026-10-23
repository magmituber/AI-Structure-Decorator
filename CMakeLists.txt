cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(AIStructureDecorator VERSION 2.1.12)

# Build the real EditorAI engine and overlay, not the previous experimental stub.
set(SOURCES src/main.cpp src/overlay.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK is not set. Use the Geode SDK build action or install the SDK.")
endif()

if (ANDROID)
    add_compile_options(-include type_traits)
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

if (WIN32 OR APPLE OR ANDROID)
    set(IMGUI_VERSION "v1.92.4")
    CPMAddPackage("gh:matcool/gd-imgui-cocos#b93f08ccef778a53ebba09b20c347f6a63980119")
    target_link_libraries(${PROJECT_NAME} imgui-cocos)
    target_compile_definitions(${PROJECT_NAME} PRIVATE EDITORAI_HAS_IMGUI=1)
endif()

setup_geode_mod(${PROJECT_NAME})
