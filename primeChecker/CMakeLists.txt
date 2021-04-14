cmake_minimum_required(VERSION 3.2)
project(helloworld)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
#-static to make compiled binaries smaller, -s to remove debug, -Os optimizes file output size
set(CMAKE_EXE_LINKER_FLAGS "-static -s -Os")
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.cpp ${CMAKE_CURRENT_SOURCE_DIR}/PrimeChecker.cpp)

enable_testing()
add_compile_options("--coverage")
add_executable(${PROJECT_NAME}-Runner TestPrimeChecker.cpp ${CMAKE_CURRENT_SOURCE_DIR}/PrimeChecker.cpp)
target_link_libraries(helloworld-runner gcov)
add_test(NAME ${PROJECT_NAME}-Runner COMMAND ${PROJECT_NAME}-Runner)
