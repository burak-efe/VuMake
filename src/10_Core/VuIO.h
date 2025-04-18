#pragma once
#include <filesystem>
#include <fstream>
#include <span>
#include <chrono>

namespace Vu
{
    inline std::vector<char> readFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        auto fileSize = file.tellg();

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


    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;

        // _popen runs the command and opens a pipe for reading its stdout
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
        if (!pipe) {
            throw std::runtime_error("_popen() failed!");
        }

        // Read the output of the command line by line
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        return result;
    }
}
