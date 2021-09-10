cmake_minimum_required(VERSION 3.16)

project(treeParser)

add_compile_options(-std=c++11)

add_executable(treeParser treeParser.cpp)

install(TARGETS treeParser)

enable_testing()
add_test(NAME treeParser COMMAND treeParser ${CMAKE_SOURCE_DIR}/J48-xplacer.txt)
