
# Unit test example
spawn_test_target(target ${CMAKE_CURRENT_SOURCE_DIR}/test_example.cpp)
add_test(NAME ${target} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target})
