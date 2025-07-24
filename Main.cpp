#include <cstdio>    // for puts
#include <cstdlib>   // for EXIT_SUCCESS
#include <exception> // for exception
#include <iostream>  // for char_traits, basic_ostream
#include <memory>    // for make_unique, unique_ptr

#include "01_InnerCore/VuLogger.h" // for LogLevel, Logger
#include "13_Scenes/Scene_GLTF_Load.h"
#include "GetTimeSinceProcessStart.h"

int
main(int argc, char* argv[]) {

  std::cout << "App Start Time: " << GetTimeSinceProcessStart() * 1000 << " millisecond" << std::endl;
  Vu::Logger::SetLevel(Vu::LogLevel::Trace);
  auto scene0 = std::make_unique<Vu::Scene_GLTF_Load>();

  try {
    scene0->run();
  } catch (const std::exception& e) { std::puts(e.what()); }
  return EXIT_SUCCESS;
}
