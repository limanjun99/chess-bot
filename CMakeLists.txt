cmake_minimum_required(VERSION 3.24)

project(
  chessBot
  VERSION 0.0.2
  LANGUAGES CXX
)

set(g_test_output_directory "${CMAKE_BINARY_DIR}/tests")
set(g_binary_output_directory "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(src)
add_subdirectory(test)
