#include <iostream>

#include "bot.h"
#include "config.h"
#include "lichess.h"
#include "logger.h"

int main() {
  try {
    Config config{};
    Lichess lichess{config};
    Bot bot{config, lichess};
    bot.listen();
  } catch (const char* error) {
    Logger::error() << error << "\n";
    return 0;
  }
}