#include "logger.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "config.h"

std::atomic<bool> Logger::initialized{false};
std::unique_ptr<Logger> Logger::singleton{nullptr};

Logger& Logger::get() {
  if (!Logger::initialized.load(std::memory_order::acquire)) {
    throw std::runtime_error{"Logger has not been initialized."};
  }
  return *Logger::singleton;
}

void Logger::initialize(const Config& config) {
  auto was_initialized = false;
  Logger::initialized.compare_exchange_strong(was_initialized, true, std::memory_order::acq_rel);
  if (was_initialized) {
    throw std::runtime_error{"Logger can only be initialized once."};
  }

  auto output_stream = [&] -> std::unique_ptr<std::ostream> {
    if (const auto& path = config.get_log_path()) {
      return std::make_unique<std::ofstream>(path.value());
    } else {
      return std::make_unique<std::ostream>(std::cout.rdbuf());
    }
  }();
  // Using new to access private constructor.
  Logger::singleton = std::unique_ptr<Logger>{new Logger{std::move(output_stream), config.get_log_level()}};
  Logger::initialized.store(true, std::memory_order::release);
}

void Logger::debug(std::string_view message) {
  queue_message(Level::Debug, std::string{debug_prefix} + std::string{message});
}

void Logger::info(std::string_view message) {
  queue_message(Level::Info, std::string{info_prefix} + std::string{message});
}

void Logger::warn(std::string_view message) {
  queue_message(Level::Warn, std::string{warn_prefix} + std::string{message});
}

void Logger::error(std::string_view message) {
  queue_message(Level::Error, std::string{error_prefix} + std::string{message});
}

Logger::Level Logger::parse_level(std::string_view level) {
  if (level == "debug") return Level::Debug;
  if (level == "info") return Level::Info;
  if (level == "warn") return Level::Warn;
  if (level == "error") return Level::Error;
  const auto error_message = std::format("Invalid log level '{}': Must be one of: debug, info, warn, error.", level);
  throw std::runtime_error{error_message};
}

Logger::~Logger() {
  std::unique_lock lock{mutex};
  stop_flag = true;
  lock.unlock();
  cond_var.notify_one();
  if (log_thread.joinable()) log_thread.join();
}

Logger::Logger(std::unique_ptr<std::ostream> out, Level level)
    : out{std::move(out)}, log_level{level}, messages{}, cond_var{}, mutex{}, stop_flag{false}, log_thread{} {
  log_thread = std::thread([this]() { listen_and_log(); });
}

void Logger::listen_and_log() {
  while (true) {
    auto lock = std::unique_lock{mutex};
    cond_var.wait(lock, [this]() { return !messages.empty() || stop_flag; });

    while (!messages.empty()) {
      const auto message = std::move(messages.front());
      messages.pop();
      *out << message << std::endl;  // Flush after each message.
    }

    if (stop_flag) break;
  }
}

void Logger::queue_message(Level level, std::string message) {
  if (std::to_underlying(level) < std::to_underlying(log_level)) return;
  const auto lock = std::lock_guard{mutex};
  messages.push(std::move(message));
  cond_var.notify_one();
}