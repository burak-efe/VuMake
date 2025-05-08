#include <cstdlib>                     // for EXIT_SUCCESS
#include <cstdio>                      // for puts
#include <exception>                   // for exception
#include <iostream>                    // for char_traits, basic_ostream
#include <memory>                      // for make_unique, unique_ptr

#include "08_LangUtils/VuLogger.h"     // for LogLevel, Logger
//#include "30_Scenes/Scene0.h"          // for Scene0

#include "GetTimeSinceProcessStart.h"
#include "12_VuMakeCore/VuInstance.h"


int main(int argc, char* argv[])
{

    auto instance = Vu::VuInstance::make(true,{},{});




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
