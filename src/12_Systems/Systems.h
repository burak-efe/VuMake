#pragma once
#include <cstdint>

namespace Vu {
struct Camera;
struct Spinn;
struct MeshRenderer;
struct Transform;
struct VuRenderer;

void drawMesh(VuRenderer& vuRenderer, Transform& transform, const MeshRenderer& meshRenderer);

void spinn(const VuRenderer& vuRenderer, Transform& trs, const Spinn& spin);

void drawSpinUI(uint32_t elemID, Spinn& spinn);

void cameraFlySystem(VuRenderer& vuRenderer, Transform& trs, Camera& cam);

// inline flecs::system AddTransformUISystem(flecs::world& world)
// {
//     return world.system<Transform>("trsUI")
//                 .each([](flecs::iter& it, size_t index, Transform& trs)
//                 {
//                     auto e = it.entity(index);
//                     // bool open = false;
//                     // if (index == 0) {
//                     // }
//                     // open = ImGui::CollapsingHeader("Transform Components");
//                     //if (open) {
//
//                     ImGui::Separator();
//                     ImGui::Text("%s", e.name().c_str());
//                     ImGui::SliderFloat3(std::format("Position ##{0}", e.id()).c_str(),
//                                         &trs.position.x, -8.0f, 8.0f);
//                     // ImGui::Text(std::format("Rotation {0:}", glm::to_string(trs.rotation)).c_str());
//                     // ImGui::Text(std::format("Scale {0:}", glm::to_string(trs.scale)).c_str());
//                     //}
//                 });
// }
//
// inline flecs::system AddCameraUISystem(flecs::world& world)
// {
//     return world.system<Camera>("camUI")
//                 .each([](flecs::entity e, Camera& cam)
//                 {
//                     if (ImGui::CollapsingHeader("Camera Components"))
//                     {
//                         ImGui::Separator();
//                         ImGui::Text("%s\n", e.name().c_str());
//                         ImGui::SliderFloat(std::format("FOV##{0}", e.id()).c_str(), &cam.fov, 20.0f, 140.0f);
//                     }
//                 });
// }

} // namespace Vu
