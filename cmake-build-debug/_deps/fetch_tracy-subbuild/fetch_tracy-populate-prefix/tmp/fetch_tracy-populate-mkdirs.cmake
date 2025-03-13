# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-src")
  file(MAKE_DIRECTORY "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-src")
endif()
file(MAKE_DIRECTORY
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-build"
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix"
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/tmp"
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/src/fetch_tracy-populate-stamp"
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/src"
  "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/src/fetch_tracy-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/src/fetch_tracy-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Dev/Vulkan/VuMake/cmake-build-debug/_deps/fetch_tracy-subbuild/fetch_tracy-populate-prefix/src/fetch_tracy-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
