#pragma once

enum class OS {
    UNKNOWN,
    WINDOWS,
    LINUX,
    MACOS,
    IOS,
    ANDROID,
};

enum class Architecture {
    UNKNOWN,
    X86_32,
    X86_64,
    AARCH32,
    AARCH64,
};


consteval OS getCurrentOS() {
#if defined(_WIN32) // MSVC, GCC, Clang (Windows, 32 or 64-bit)
return  OS::WINDOWS;
#endif

#if defined(__APPLE__) && defined(__MACH__) // GCC, Clang (macOS/iOS)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
return OS::IOS;
#else
    return OS::MACOS;
#endif
#endif

#if defined(__ANDROID__) // GCC, Clang (Android)
    return OS::ANDROID;
#endif

#if defined(__linux__) && !defined(__ANDROID__) // GCC, Clang (Linux, but not Android)
    return OS::LINUX;
#endif

    return OS::UNKNOWN;
};


consteval Architecture getCurrentArchitecture() {
#if defined(_M_X64)  || defined(__x86_64__)  || defined(_M_AMD64)      // MSVC (64-bit), GCC Clang (64-bit), MSVC (alternative name)



    return Architecture::X86_64;
#endif

#if defined(_M_IX86)  || defined(__i386__)   // MSVC (32-bit x86) GCC, Clang (32-bit x86)
    return Architecture::X86_32;
#endif

#if defined(__aarch64__) || defined(_M_ARM64)     // GCC, Clang (ARM 64-bit). MSVC (ARM 64-bit)
    return Architecture::AARCH64;
#endif

#if defined(__arm__) || defined(_M_ARM)  // GCC, Clang (ARM 32-bit) MSVC (ARM 32-bit)
    return Architecture::AARCH32;
#endif

    return Architecture::UNKNOWN;
}


inline void appendTargetVariablePath(std::filesystem::path& path) {
    std::string apnd;

    switch (OS os = getCurrentOS()) {
        case OS::UNKNOWN:
            apnd = "unknown";
            break;
        case OS::WINDOWS:
            apnd = "windows";
            break;
        case OS::LINUX:
            apnd = "linux";
            break;
        case OS::MACOS:
            apnd = "macos";
            break;
        case OS::IOS:
            apnd = "ios";
            break;
        case OS::ANDROID:
            apnd = "android";
            break;
    }

    switch (Architecture architecture = getCurrentArchitecture()) {
        case Architecture::UNKNOWN:
            apnd += "-unknown";
            break;
        case Architecture::X86_32:
            apnd += "-x86_32";
            break;
        case Architecture::X86_64:
            apnd += "-x86_64";
            break;
        case Architecture::AARCH32:
            apnd += "-aarch32";
            break;
        case Architecture::AARCH64:
            apnd += "-aarch64";
            break;
    }

    path /= apnd;
}


inline void appendTargetVariableExtension(std::filesystem::path& path) {
    switch (auto os = getCurrentOS()) {
        case OS::UNKNOWN:
            break;
        case OS::WINDOWS:
            path.replace_extension(".exe");
            break;
        case OS::LINUX:
            break;
        case OS::MACOS:
            break;
        case OS::IOS:
            break;
        case OS::ANDROID:
            break;
    }
}
