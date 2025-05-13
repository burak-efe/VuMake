#include <cstdio>    // for puts
#include <cstdlib>   // for EXIT_SUCCESS
#include <exception> // for exception
#include <iostream>  // for char_traits, basic_ostream
#include <memory>    // for make_unique, unique_ptr

#include "01_InnerCore/VuLogger.h" // for LogLevel, Logger
// #include "12_VuMakeCore/VuInstance.h"
// #include "30_Scenes/Scene0.h"          // for Scene0


#include "04_Crust/VuRenderer.h"


int
main(int argc, char* argv[]) {

  Vu::VuRendererCreateInfo info {};

  Vu::VuRenderer renderer{info};


  // std::cout << "App Start Time: " << GetTimeSinceProcessStart() * 1000 << " millisecond" << std::endl;
  // Vu::Logger::SetLevel(Vu::LogLevel::Trace);
  // auto scene0 = std::make_unique<Vu::Scene0>();
  //
  // try
  // {
  //     scene0->Run();
  // }
  // catch (const std::exception& e)
  // {
  //     std::puts(e.what());
  // }
  return EXIT_SUCCESS;
}
