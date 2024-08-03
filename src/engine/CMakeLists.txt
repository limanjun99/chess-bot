# Library

add_library(
  chess_engine
  src/engine_impl.cpp
  src/engine.cpp
  src/evaluation.cpp
  src/history_heuristic.cpp
  src/killer_moves.cpp
  src/move_priority.cpp
  src/search_impl.cpp
  src/search.cpp
  src/time_management.cpp
  src/transposition_table.cpp
  src/uci.cpp
)

target_compile_features(chess_engine PRIVATE cxx_std_23)
target_include_directories(chess_engine PRIVATE include/chess_engine)
target_include_directories(chess_engine PUBLIC include)

target_compile_options(chess_engine PRIVATE -Wall -Wextra -O3)

target_link_libraries(chess_engine PUBLIC chess)


# Tests

enable_testing()

add_executable(
  chess_engine_tests
  src/engine.test.cpp
)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(chess_engine_tests PRIVATE chess_engine)
target_link_libraries(chess_engine_tests PRIVATE chess)
target_link_libraries(chess_engine_tests PRIVATE GTest::gtest_main)

target_compile_features(chess_engine_tests PRIVATE cxx_std_23)
target_compile_options(chess_engine_tests PRIVATE -Wall -Wextra)
set_target_properties(chess_engine_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${g_test_output_directory})

include(GoogleTest)
gtest_discover_tests(chess_engine_tests)
