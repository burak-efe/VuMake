#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"
#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "Common.h"
#include "MonkeScene.h"

#include "spirv_reflect.h"
#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy_alloc.h"

#include "daw/json/daw_json_link.h"

struct Thing {
    uint32 a;
    uint32 b;
};

namespace daw::json {
    template<>
    struct json_data_contract<Thing> {
        using type = json_member_list<
            json_number<"a", uint32>,
            json_number<"b", uint32>
        >;

        static auto to_json_data(Thing const& v) {
            return std::forward_as_tuple(v.a, v.b);
        }
    };
}

int main(int argc, char* argv[]) {
    MonkeScene scen;
    scen.Run();

    // Thing toWrite{21, 23};
    // std::string s = daw::json::to_json(toWrite);
    //
    // std::string filename("assets/materials/tmp.json");
    // std::fstream file_out;
    //
    // file_out.open(filename, std::ios_base::out);
    //
    // if (!file_out.is_open()) {
    //     std::cout << "failed to open " << filename << '\n';
    // } else {
    //     file_out << s.c_str() << std::endl;
    //     std::cout << "Done Writing!" << std::endl;
    // }


    return EXIT_SUCCESS;
}
