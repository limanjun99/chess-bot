#pragma once

#include <fstream>
#include <iostream>

enum class LogLevel { Info = 0, Warn = 1, Error = 2 };

class Logger {
public:
  // Usage: `Logger::info() << "Information";`
  static std::ostream& info();

  // Usage: `Logger::warn() << "Warning";`
  static std::ostream& warn();

  // Usage: `Logger::error() << "Error";`
  static std::ostream& error();

  // Flush the buffered output.
  static void flush();

private:
  std::ostream& out;

  Logger(std::ostream& out);

  // Returns the sole instance of this singleton Logger.
  static Logger& instance();
};