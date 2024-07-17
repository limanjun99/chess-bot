add_library(
  chess_engine
  src/engine.cpp
  src/evaluation.cpp
  src/history_heuristic.cpp
  src/killer_moves.cpp
  src/transposition_table.cpp
  src/uci.cpp
)

target_compile_features(chess_engine PRIVATE cxx_std_23)
target_include_directories(chess_engine PRIVATE include/chess_engine)
target_include_directories(chess_engine PUBLIC include)

target_compile_options(chess_engine PRIVATE -Wall -Wextra -Wpedantic -O3)

target_link_libraries(chess_engine PUBLIC chess)
