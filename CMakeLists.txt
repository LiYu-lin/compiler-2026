# 在CMAKE_CXX_FLAGS设置前添加MSVC检查
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
    # 对于MinGW/GCC启用并行编译
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
    add_compile_options(-march=native)
endif()

cmake_minimum_required(VERSION 3.12.0)
project(SysY2022 VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Module path
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

    # C Test
    include(CTest)
    enable_testing()
endif()

include(AuxFunctions)
include(GNUInstallDirs)

# Subdirectories
add_subdirectory(lib)
add_subdirectory(include)

get_property(lib_source_files GLOBAL PROPERTY PROJECT_LIBRARY_SRCS)
get_property(lib_header_files GLOBAL PROPERTY PROJECT_LIBRARY_HEADERS)

add_library(${PROJECT_NAME} STATIC ${lib_source_files})

# Target
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(sysy_compiler ${CMAKE_CURRENT_SOURCE_DIR}/tools/sysy_compiler.cpp)
target_link_libraries(sysy_compiler PUBLIC ${PROJECT_NAME})
target_include_directories(sysy_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS sysy_compiler
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
    AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
