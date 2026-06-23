cmake_minimum_required(VERSION 3.12.0)
project(SysY2026 VERSION 0.1.0 LANGUAGES C CXX)

option(ENABLE_OPTIMIZATION "Enable project optimizations (adds -O2 or /O2 and defines)" OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
    if(MINGW OR WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

    include(CTest)
    enable_testing()
endif()

include(AuxFunctions)
include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(include)

get_property(lib_source_files GLOBAL PROPERTY PROJECT_LIBRARY_SRCS)
get_property(lib_header_files GLOBAL PROPERTY PROJECT_LIBRARY_HEADERS)

add_library(${PROJECT_NAME} STATIC ${lib_source_files})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Parse
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GenIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IR
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IR/Value
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IR/opt
    ${CMAKE_CURRENT_SOURCE_DIR}/include/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/include/backend/RiscVOperand
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

add_executable(compiler ${CMAKE_CURRENT_SOURCE_DIR}/tools/sysy_compiler.cpp)
target_link_libraries(compiler PUBLIC ${PROJECT_NAME})
target_include_directories(compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(sysy_compiler ${CMAKE_CURRENT_SOURCE_DIR}/tools/sysy_compiler.cpp)
target_link_libraries(sysy_compiler PUBLIC ${PROJECT_NAME})
target_include_directories(sysy_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ENABLE_OPTIMIZATION)
    if(MSVC)
        set(OPT_FLAGS "/O2")
    else()
        set(OPT_FLAGS "-O2")
    endif()

    add_compile_definitions(ENABLE_OPTIMIZATION)
    target_compile_options(${PROJECT_NAME} PRIVATE ${OPT_FLAGS})
    target_compile_options(compiler PRIVATE ${OPT_FLAGS})
    target_compile_options(sysy_compiler PRIVATE ${OPT_FLAGS})
endif()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS compiler sysy_compiler
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
    AND BUILD_TESTING)
    add_subdirectory(tests)
endif()