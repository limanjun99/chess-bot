#include <iostream>

#include "config.h"
#include "lichess.h"
#include "logger.h"

int main() {
  try {
    Config config{};
    Lichess lichess{config};
    lichess.listen();
  } catch (const char* error) {
    Logger::error() << error << "\n";
    return 0;
  }
}