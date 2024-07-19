add_executable(
  lichess_bot
  bot/bot_state.cpp
  bot/challenge_accepted_state.cpp
  bot/challenge_issued_state.cpp
  bot/game_state.cpp
  bot/idle_state.cpp
  bot.cpp
  config.cpp
  game_handler.cpp
  lichess.cpp
  logger.cpp
  main.cpp
  ndjson.cpp)

target_compile_features(lichess_bot PRIVATE cxx_std_20)
target_compile_options(lichess_bot PRIVATE -Wall -Wextra -Wpedantic -O3)
set_target_properties(lichess_bot PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${g_binary_output_directory})

target_link_libraries(lichess_bot PRIVATE chess_engine)
target_link_libraries(lichess_bot PRIVATE chess)

find_package(cpr CONFIG REQUIRED)
target_link_libraries(lichess_bot PRIVATE cpr::cpr)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(lichess_bot PRIVATE nlohmann_json::nlohmann_json)
