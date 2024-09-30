# VuMake: A Terrible Vulkan Renderer

This is a terrible vulkan renderer, barely functioning.
It's a CMAKE project with C++ 23 required.
(c++ 23 requreid because I used println twice 💀)

### Capabilites
- ✅ Render one object as unlit
- ✅ Imgui 
- ✅ Depth Buffer 
- ✅ Push Constants 
- 👷‍♂️ ECS

<details> 
  <summary>
  Incapabilities (there is only few)
  </summary>
  
- ❌ Vulkan Sync Abstraction
- ❌ Textures
- ❌ Normals / Tangents
- ❌ Normal - Bump Mappping
- ❌ Directional Ligths
- ❌ Point Ligths
- ❌ Spot Ligths
- ❌ Area Ligths
- ❌ PBR
- ❌ Scene-Level Representation
- ❌ Ray Traced GI
- ❌ Path Traced Gi
- ❌ SSGI
- ❌ Voxel GI
- ❌ SDFGI
- ❌ Probe Based GI
- ❌ DDGI
- ❌ ReSTIR GI
- ❌ Surfels GI
- ❌ Radiance Cascades GI
- ❌ Skinned Geometry
- ❌ Animations
- ❌ Deffered Rendering
- ❌ Tiled Forward Rendering
- ❌ FXAA - TAA - SMAA - MSAA
- ❌ FSR - DLSS - XESS
- ❌ PSO Cache System
- ❌ Bindless Rendering
- ❌ Ligth Map Baking
- ❌ Compute Shaders
- ❌ Subdivison Surfaces
- ❌ Mesh Shaders
- ❌ Shadow Maps
- ❌ Occlusion Culling
- ❌ Screen Space Shadows
- ❌ LOD System
- ❌ HDR
- ❌ Cube Maps- Sky Maps
- ❌ Tonemapping
- ❌ Bloom
- ❌ Twenty Other Post Process Effects
- ❌ SDF - Volume Rendering
- ❌ Debug View
- ❌ Tessallation
- ❌ Geometry Shaders
- ❌ Shader Editor
- ❌ HLSL Support
- ❌ SSAO / HBAO
- ❌ SSR - SSSR
- ❌ RayTraycing
- ❌ Sub-Surface Scattering
- ❌ GPU Driven Rendering
- ❌ Work Graphs
- ❌ Visibility Buffer
- ❌ Compute Rasterizer
- ❌ FBX - OBJ Support
- ❌ Font Rendering
- ❌ Instanceing
- ❌ Batching
- ❌ Lens Flares
- ❌ Particles
- ❌ Hair Rendering
- ❌ Texture Streaming
- ❌ Variable Rate Shading
- ❌ Decals
- ❌ Frame Profiler - Debugger
- ❌ Displacement Mapping
- ❌ Morph Target
- ❌ Planar Reflections
- ❌ Volumetric Ligths
- ❌ Water Rendering
- ❌ Outline Shaders
- ❌ Contact SHadows
- ❌ Caustics
- ❌ Render Graph
</details>

### It Uses:
- Vulkan Memory Allocator
- IMGUI
- FastGLTF
- GLFW
- GLM
- flecs
- Vk-Bootstrap

### Vulkan Notes
- It uses Dynamic Rendering so there is no Render Pass (It's a good thing)

### How to build and run
- have vulkan sdk
- have some IDE that CMAKE support
- clone the repo
- open the project as a CMAKE project
- hit the build button
- compile shaders with vulkan sdk (sorry about that)
- done

### Hardest parts of this project in most to least  order
- Adding Some Dependencies to CMAKE
- Understanding Vulkan Descriptors
- Initilazing vulkan
- Loading GLTF
- Adding Imgui
- Using Vulkan Mem Allocator
