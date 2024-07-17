#include <iostream>

#include "engine_cli.h"

int main() {
  EngineCli engine_cli{std::cin, std::cout};
  engine_cli.start();
}