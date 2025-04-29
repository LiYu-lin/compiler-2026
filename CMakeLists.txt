cmake_minimum_required(VERSION 3.10.0)
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

add_library(${PROJECT_NAME} SHARED ${lib_source_files})

# Boost
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::filesystem)
include_directories(${Boost_INCLUDE_DIRS})

if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

# Target
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
    AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
