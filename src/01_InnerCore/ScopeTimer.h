#pragma once

#include <chrono>
#include <iostream>
#include <source_location>

// RAII function timer, prints time on destruction
struct ScopeTimer {
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start = std::chrono::high_resolution_clock::now();
  std::source_location                                        m_sourceLocation;

  explicit ScopeTimer(const std::source_location& location = std::source_location::current()) :
      m_sourceLocation(location) {}

  ~ScopeTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
    std::cout << "[TIMER] " << m_sourceLocation.function_name() << ": " << ms << "ms" << std::endl;
  }
};
