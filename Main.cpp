#include "08_LangUtils/VuLogger.h"
#include "08_LangUtils/collections/AllocatorManager.h"
#include "08_LangUtils/collections/VuList.h"
#include "30_Scenes/Scene0.h"


#define GTSPS_IMPLEMENTATION
#include "GetTimeSinceProcessStart.h"

int main(int argc, char* argv[])

{
    std::cout << "App Start Time: " << GetTimeSinceProcessStart() * 1000 << " millisecond" << std::endl;

    Vu::Logger::SetLevel(Vu::LogLevel::Trace);
    AllocatorManager::init({AllocatorHandle::Default()});

    Vu::Scene0* scene0 = new Vu::Scene0();

    VuList<u32> list(1, AllocatorHandle::Default());

    try
    {
        scene0->Run();
    }
    catch (const std::exception& e)
    {
        std::puts(e.what());
        system("pause");
    }

    list.uninit();

    AllocatorManager::uninit({AllocatorHandle::Default()});

    return EXIT_SUCCESS;
}
