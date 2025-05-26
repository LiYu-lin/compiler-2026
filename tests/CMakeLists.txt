#test_lexer
spawn_test_target(test_lexer ${CMAKE_CURRENT_SOURCE_DIR}/test_lexer.cpp)
add_test(NAME ${test_lexer} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_lexer})
#test_parser1
spawn_test_target(test_parser1 ${CMAKE_CURRENT_SOURCE_DIR}/test_parser1.cpp)
add_test(NAME ${test_parser1} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_parser1})
#test_parser2
spawn_test_target(test_parser2 ${CMAKE_CURRENT_SOURCE_DIR}/test_parser2.cpp)
add_test(NAME ${test_parser2} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_parser2})
