cmake_minimum_required(VERSION 2.8)
project(terminal_tv)

set(OpenCV_STATIC ON)

SET("OpenCV_DIR" "/usr/share/OpenCV")
find_package( OpenCV REQUIRED )

MESSAGE(STATUS "Include dirs ${OpenCV_INCLUDE_DIRS}")
MESSAGE(STATUS "LINK LIBRARIES ${OpenCV_LIBS}")

add_executable(
        terminal_tv
        src/main.cpp
)

target_link_libraries(terminal_tv ${OpenCV_LIBS})
