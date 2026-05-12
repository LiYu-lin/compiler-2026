spawn_test_target(test_backend ${CMAKE_CURRENT_SOURCE_DIR}/test_backend.cpp)
add_test(NAME ${test_backend} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_backend})
get_filename_component(_toolchain_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
set_tests_properties(${test_backend} PROPERTIES
    ENVIRONMENT "PATH=${_toolchain_bin_dir};$ENV{PATH}"
)

spawn_test_target(test_driver ${CMAKE_CURRENT_SOURCE_DIR}/test_driver.cpp)
target_compile_definitions(${test_driver} PRIVATE SYSY_COMPILER_PATH="$<TARGET_FILE:compiler>")
add_test(NAME ${test_driver} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_driver})
set_tests_properties(${test_driver} PROPERTIES
    ENVIRONMENT "PATH=${_toolchain_bin_dir};$ENV{PATH}"
)
