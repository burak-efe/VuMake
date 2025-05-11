#pragma once

#include <format>
#include <iostream>
#include <mutex>
#include <string_view>

namespace Vu {

enum class LogLevel {
  Trace = 0,
  Debug,
  Info,
  Warn,
  Error,
  None // No logs
};

class Logger {
public:
  static void     SetLevel(LogLevel level) { currentLevel = level; }
  static LogLevel GetLevel() { return currentLevel; }

  template <typename... Args> static void Trace(std::format_string<Args...> fmt, Args &&...args) {
    Log(LogLevel::Trace, "TRACE", fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void Debug(std::format_string<Args...> fmt, Args &&...args) {
    Log(LogLevel::Debug, "DEBUG", fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void Info(std::format_string<Args...> fmt, Args &&...args) {
    Log(LogLevel::Info, "INFO", fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void Warn(std::format_string<Args...> fmt, Args &&...args) {
    Log(LogLevel::Warn, "WARN", fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> static void Error(std::format_string<Args...> fmt, Args &&...args) {
    Log(LogLevel::Error, "ERROR", fmt, std::forward<Args>(args)...);
  }

private:
  inline static LogLevel   currentLevel;
  inline static std::mutex mutex;

  template <typename... Args>
  static void Log(LogLevel level, std::string_view levelStr, std::format_string<Args...> fmt, Args &&...args) {
    std::lock_guard lock(mutex);

    if (level < currentLevel)
      return;

    std::string   message = std::format(fmt, std::forward<Args>(args)...);
    std::ostream &stream  = (level >= LogLevel::Warn) ? std::cerr : std::cout;

    stream << "[" << levelStr << "] " << message << std::endl;
  }
};

} // namespace Vu
