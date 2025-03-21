#include "10_Core/VuCommon.h"
#include "30_Scenes/Scene0.h"

int main(int argc, char* argv[])
{
    Vu::Scene0* scene0 = new Vu::Scene0();

    try
    {
        scene0->Run();
    }
    catch (const std::exception& e)
    {
        std::puts(e.what());
        system("pause");
    }

    return EXIT_SUCCESS;
}
