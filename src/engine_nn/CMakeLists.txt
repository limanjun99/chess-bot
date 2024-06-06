add_library(
  chess_engine_nn
  src/chess_game.cpp
  src/engine.cpp)

target_compile_features(chess_engine_nn PRIVATE cxx_std_20)
target_include_directories(chess_engine_nn PRIVATE include/chess_engine_nn)
target_include_directories(chess_engine_nn PUBLIC include)

target_compile_options(chess_engine_nn PRIVATE -Wall -Wextra -Wpedantic -O3)

target_link_libraries(chess_engine_nn PUBLIC chess)