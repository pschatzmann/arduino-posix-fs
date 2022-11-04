cmake_minimum_required(VERSION 3.16)

# set the project name
project(arduino_posix_fs)

include(FetchContent)

# lots of warnings and all warnings as errors
## add_compile_options(-Wall -Wextra )
set(CMAKE_CXX_STANDARD 17)


file(GLOB_RECURSE SRC_LIST_CPP CONFIGURE_DEPENDS  "${PROJECT_SOURCE_DIR}/src/*.cpp" )

# define libraries
add_library (arduino_posix_fs ${SRC_LIST_CPP})

#target_compile_options(arduino_posix_fs PRIVATE -Wdouble-promotion)
target_compile_definitions(arduino_posix_fs PUBLIC -DIS_DESKTOP )

# define location for header files
target_include_directories(arduino_posix_fs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )
