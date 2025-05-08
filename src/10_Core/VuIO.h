#pragma once
#include <filesystem>
#include <fstream>
#include <span>
#include <chrono>

namespace Vu
{
    inline std::optional<std::vector<char>> readFile(const std::filesystem::path& path)
    {
        if (std::filesystem::exists(path) == false)
        {
            return std::nullopt;
        }
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            return std::nullopt;
        }

        std::streampos fileSize = file.tellg();

        if (fileSize == -1)
        {
            return std::nullopt;
        }

        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    inline void createFile(const std::filesystem::path& path, std::span<const uint8_t> data)
    {
        // Open file for writing in binary mode
        std::ofstream file(path, std::ios::binary | std::ios::out);

        // Check if the file was successfully opened
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        // Write data to the file
        file.write(reinterpret_cast<const char*>(data.data()), data.size());

        // Ensure the data was written successfully
        if (!file)
        {
            throw std::runtime_error("Failed to write to file: " + path.string());
        }
    }

    inline time_t getlastModifiedTime(const std::filesystem::path& path)
    {
        const auto fileTime   = std::filesystem::last_write_time(path);
        const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
        const auto time       = std::chrono::system_clock::to_time_t(systemTime);
        return time;
    }
}
