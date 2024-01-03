#include <iostream>

#include "config.h"
#include "lichess.h"

int main() {
  Config config{};
  Lichess lichess{config};
  lichess.listen();
}