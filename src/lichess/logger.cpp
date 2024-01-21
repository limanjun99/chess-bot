#include "logger.h"

std::ostream& Logger::info() {
  Logger& logger = Logger::instance();
  logger.out << "[info] ";
  return logger.out;
}

std::ostream& Logger::warn() {
  Logger& logger = Logger::instance();
  logger.out << "[warn] ";
  return logger.out;
}

std::ostream& Logger::error() {
  Logger& logger = Logger::instance();
  logger.out << "[error] ";
  return logger.out;
}

void Logger::flush() {
  Logger& logger = Logger::instance();
  logger.out.flush();
}

Logger::Logger(std::ostream& out) : out{out} {}

Logger& Logger::instance() {
  static Logger logger{std::cout};
  return logger;
}
