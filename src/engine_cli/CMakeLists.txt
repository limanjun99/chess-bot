# Everything except the main executable is extracted into a library,
# so that it can be linked against by the test executable too.

add_library(engine_cli_lib
  commands/command.cpp
  commands/debug_command.cpp
  commands/go_command.cpp
  commands/new_game_command.cpp
  commands/parsing.cpp
  commands/position_command.cpp
  commands/quit_command.cpp
  commands/ready_command.cpp
  commands/stop_command.cpp
  commands/uci_command.cpp
  engine_cli.cpp
  outputs/best_move_output.cpp
  outputs/error_output.cpp
  outputs/id_output.cpp
  outputs/output.cpp
  outputs/ready_output.cpp
  outputs/uciok_output.cpp
  uci_io.cpp
)

target_compile_features(engine_cli_lib PRIVATE cxx_std_23)
target_compile_options(engine_cli_lib PRIVATE -Wall -Wextra -Wpedantic -O3)

target_link_libraries(engine_cli_lib PRIVATE chess_engine)
target_link_libraries(engine_cli_lib PRIVATE chess)

# Main executable

add_executable(engine_cli
  main.cpp
)

target_compile_features(engine_cli PRIVATE cxx_std_23)
target_compile_options(engine_cli PRIVATE -Wall -Wextra -Wpedantic -O3)

target_link_libraries(engine_cli PRIVATE engine_cli_lib)
target_link_libraries(engine_cli PRIVATE chess_engine)
target_link_libraries(engine_cli PRIVATE chess)

# Testing

enable_testing()

add_executable(engine_cli_tests
  commands/debug_command.test.cpp
  commands/go_command.test.cpp
  commands/new_game_command.test.cpp
  commands/parsing.test.cpp
  commands/position_command.test.cpp
  commands/quit_command.test.cpp
  commands/ready_command.test.cpp
  commands/stop_command.test.cpp
  commands/uci_command.test.cpp
  engine_cli.test.cpp
  uci_io.test.cpp
  util.test.cpp
)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(engine_cli_tests PRIVATE engine_cli_lib)
target_link_libraries(engine_cli_tests PRIVATE chess_engine)
target_link_libraries(engine_cli_tests PRIVATE chess)
target_link_libraries(engine_cli_tests PRIVATE GTest::gtest_main)

target_compile_features(engine_cli_tests PRIVATE cxx_std_23)
target_compile_options(engine_cli_tests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(engine_cli_tests)
