cmake_minimum_required(VERSION 3.5)  
project(PowderDataProcessing VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_resources(RESOURCE_ADDED resources.qrc)

add_executable(${PROJECT_NAME}
    powder_processing.cpp
    ${RESOURCE_ADDED}
)

# 🎯 Platform-specific settings (Windows only)
if(WIN32)

    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_STATIC)

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
