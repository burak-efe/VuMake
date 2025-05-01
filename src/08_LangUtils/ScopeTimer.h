#pragma once

#include <chrono>
#include <iostream>
#include <source_location>

// RAII function timer
struct ScopeTimer
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    std::source_location                                        sourceLocation;

    explicit ScopeTimer(const std::source_location& location = std::source_location::current())
        : sourceLocation(location)
    {
    }


    ~ScopeTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "[TIMER] " << sourceLocation.function_name() << ": " << ms << "ms" << std::endl;
    }
};
