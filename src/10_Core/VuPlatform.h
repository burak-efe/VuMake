#pragma once

#if defined(_WIN32) // MSVC, GCC, Clang (Windows, 32 or 64-bit)
#define OS_WINDOWS
#endif

#if defined(__APPLE__) && defined(__MACH__) // GCC, Clang (macOS/iOS)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define OS_IOS
#else
#define OS_MAC
#endif
#endif

#if defined(__ANDROID__) // GCC, Clang (Android)
#define OS_ANDROID
#endif

#if defined(__linux__) && !defined(__ANDROID__) // GCC, Clang (Linux, but not Android)
#define OS_LINUX
#endif

#if !defined(OS_WINDOWS) && !defined(OS_LINUX) && !defined(OS_MAC) && !defined(OS_ANDROID) && !defined(OS_IOS)
#define OS_UNKNOWN
#endif

//--------------------------------
// Architecture Detection
//--------------------------------

#if defined(_M_X64)  || defined(__x86_64__)  || defined(_M_AMD64)      // MSVC (64-bit), GCC Clang (64-bit), MSVC (alternative name)
#define ARCH_X86_64
#endif

#if defined(_M_IX86)  || defined(__i386__)   // MSVC (32-bit x86) GCC, Clang (32-bit x86)
   #define ARCH_X86_32
#endif

#if defined(__aarch64__) || defined(_M_ARM64)     // GCC, Clang (ARM 64-bit). MSVC (ARM 64-bit)
   #define ARCH_ARM_64
#endif

#if defined(__arm__) || defined(_M_ARM)  // GCC, Clang (ARM 32-bit) MSVC (ARM 32-bit)
   #define ARCH_ARM_32
#endif

#if !defined(ARCH_X86_64) && !defined(ARCH_X86_32) && !defined(ARCH_ARM_64) && !defined(ARCH_ARM_32)
   #define ARCH_UNKNOWN
#endif
#include <filesystem>


#if defined(OS_WINDOWS) && defined(ARCH_X86_64)
#define PLATFORM_SPECIFIC_PATH "/windows-x86_64"
#elif  defined(OS_LINUX) && defined(ARCH_X86_32)
#define PLATFORM_SPECIFIC_PATH /linux-x86_64
#else
#endif


inline void addTagetDependentPath(std::filesystem::path& path)
{
#if defined(OS_WINDOWS) && defined(ARCH_X86_64)
    path.append("/windows-x86_64");
#elif  defined(OS_LINUX) && defined(ARCH_X86_32)
   path.append("/linux-x86_64");
#else
   UNSUPPORTED TARGET
#endif
}
