
# Unit test example
spawn_test_target(test_lexer ${CMAKE_CURRENT_SOURCE_DIR}/test_lexer.cpp)
add_test(NAME ${test_lexer} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target})

spawn_test_target(test_parser ${CMAKE_CURRENT_SOURCE_DIR}/test_parser.cpp)
add_test(NAME ${test_parser} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target})
