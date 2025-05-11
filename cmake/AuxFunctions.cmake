function(add_library_source)
    get_property(lib_files GLOBAL PROPERTY PROJECT_LIBRARY_SRCS)
    foreach(file ${ARGN})
        list(APPEND lib_files ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        set(PROJECT_LIBRARY_SRCS ${lib_files})
    endforeach()
    set_property(GLOBAL PROPERTY PROJECT_LIBRARY_SRCS ${lib_files})
endfunction()

function(add_library_header)
    get_property(lib_files GLOBAL PROPERTY PROJECT_LIBRARY_HEADERS)
    foreach(file ${ARGN})
        list(APPEND lib_files ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        set(PROJECT_LIBRARY_HEADERS ${lib_files})
    endforeach()
    set_property(GLOBAL PROPERTY PROJECT_LIBRARY_HEADERS ${lib_files})
endfunction()

function(spawn_test_target target_name file)
    get_filename_component(name ${file} NAME)
    string(REPLACE ".cpp" "" name ${name})
    add_executable(${name} ${file})
    target_link_libraries(${name}
        PUBLIC
        SysY2022
    )
    target_include_directories(${name}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    )
    set(${target_name} ${name})
    set(${target_name} ${${target_name}} PARENT_SCOPE)
endfunction()
