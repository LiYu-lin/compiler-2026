add_library_source(
    recycle.cpp
)

# 将utils源文件添加到全局库源文件列表
set_property(GLOBAL APPEND PROPERTY PROJECT_LIBRARY_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/recycle.cpp)