#include <iostream>
#include <stdexcept>

#include "bot.h"
#include "config.h"
#include "lichess.h"
#include "logger.h"

int main() {
  try {
    auto config_file = std::ifstream{".env"};
    const auto config = Config::from_stream(config_file);
    Logger::initialize(config);
    const auto lichess = Lichess{config};
    auto bot = Bot{config, lichess};
    bot.listen();
  } catch (const std::runtime_error& error) {
    std::cerr << error.what();
    return 1;
  }
}