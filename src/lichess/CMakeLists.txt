add_executable(
  lichess_bot
  bot.cpp
  config.cpp
  game_handler.cpp
  lichess.cpp
  logger.cpp
  main.cpp
  ndjson.cpp)

target_compile_features(lichess_bot PRIVATE cxx_std_20)
target_compile_options(lichess_bot PRIVATE -Wall -Wextra -Wpedantic -O3)

target_link_libraries(lichess_bot PRIVATE chess)
target_link_libraries(lichess_bot PRIVATE chess_engine)

include(FetchContent)
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8
)# Replace with your desired git commit from:
 # https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)
target_link_libraries(lichess_bot PRIVATE cpr::cpr)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(lichess_bot PRIVATE nlohmann_json::nlohmann_json)
