#include "08_LangUtils/VuLogger.h"
#include "30_Scenes/Scene0.h"

#define GTSPS_IMPLEMENTATION
#include "GetTimeSinceProcessStart.h"

int main(int argc, char* argv[])
{
    std::cout << "App Start Time: " << GetTimeSinceProcessStart() * 1000 << " millisecond" << std::endl;

    Vu::Logger::SetLevel(Vu::LogLevel::Trace);
    auto scene0 = std::make_unique<Vu::Scene0>();

    try
    {
        scene0->Run();
    }
    catch (const std::exception& e)
    {
        std::puts(e.what());
    }


    return EXIT_SUCCESS;
}
