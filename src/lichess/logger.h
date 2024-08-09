#pragma once

#include <condition_variable>
#include <format>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>
#include <string_view>
#include <thread>

// Forward declaration.
class Config;

// A thread-safe singleton logger.
class Logger {
public:
  enum class Level { Debug = 0, Info = 1, Warn = 2, Error = 3 };
  // Creates a Logger::Level from a string. Throws a `runtime_error` if the string does not match any level.
  static Level parse_level(std::string_view level);

  static Logger& get();
  static void initialize(const Config& config);

  void debug(std::string_view message);
  void info(std::string_view message);
  void warn(std::string_view message);
  void error(std::string_view message);

  // Wow these format_...() functions actually do compile-time checks on the format string.
  // Refer to: https://stackoverflow.com/a/72795239
  template <typename... Args>
  void format_debug(std::format_string<Args...> fmt, Args&&... args);
  template <typename... Args>
  void format_info(std::format_string<Args...> fmt, Args&&... args);
  template <typename... Args>
  void format_warn(std::format_string<Args...> fmt, Args&&... args);
  template <typename... Args>
  void format_error(std::format_string<Args...> fmt, Args&&... args);

  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger();

private:
  std::unique_ptr<std::ostream> out;
  Level log_level;
  std::queue<std::string> messages;
  std::condition_variable cond_var;
  std::mutex mutex;
  bool stop_flag;
  std::thread log_thread;

  explicit Logger(std::unique_ptr<std::ostream> out, Level level);

  void listen_and_log();
  void queue_message(Level level, std::string message);

  static std::atomic<bool> initialized;
  static std::unique_ptr<Logger> singleton;

  static constexpr auto debug_prefix = std::string_view{"[debug] "};
  static constexpr auto info_prefix = std::string_view{"[info] "};
  static constexpr auto warn_prefix = std::string_view{"[warn] "};
  static constexpr auto error_prefix = std::string_view{"[error] "};
};

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

template <typename... Args>
void Logger::format_debug(std::format_string<Args...> fmt, Args&&... args) {
  queue_message(Level::Debug, std::string{debug_prefix} + std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void Logger::format_info(std::format_string<Args...> fmt, Args&&... args) {
  queue_message(Level::Info, std::string{info_prefix} + std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void Logger::format_warn(std::format_string<Args...> fmt, Args&&... args) {
  queue_message(Level::Warn, std::string{warn_prefix} + std::format(fmt, std::forward<Args>(args)...));
}
template <typename... Args>
void Logger::format_error(std::format_string<Args...> fmt, Args&&... args) {
  queue_message(Level::Error, std::string{error_prefix} + std::format(fmt, std::forward<Args>(args)...));
}