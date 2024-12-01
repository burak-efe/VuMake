# VuMake: A Terrible Vulkan Renderer

This is a terrible vulkan renderer, barely functioning.
It's a CMAKE project with C++ 23 required.
(c++ 23 requreid because I used println twice 💀)

### Capabilites
- ✅ Bindless Resources (via Descriptor indexing)
- ✅ Imgui (docking branch)
- ✅ Textures
- ✅ Push Constants
- ✅ PBR (blinn-phong)
- ✅ ECS (via flecs)
- ✅ Normals / Tangents
- ✅ Normal - Bump Mappping
- ✅ Frame Profiling (via Tracy)

<details> 
  <summary>
  Incapabilities (there is only few)
  </summary>
  
- ❌ Deffered Rendering
- ❌ Vulkan Sync Abstraction
- ❌ Directional Ligths
- ❌ Point Ligths
- ❌ Spot Ligths
- ❌ Area Ligths
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
- ❌ Tiled Forward Rendering
- ❌ FXAA - TAA - SMAA - MSAA
- ❌ FSR - DLSS - XESS
- ❌ PSO Cache System
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
- SDL3
- GLM
- flecs
- Vk-Bootstrap
- SLang
- Tracy
- Volk

### Vulkan Notes
- TODO


### How to build and run
- have vulkan sdk
- have some IDE that CMAKE support
- clone the repo
- open the project as a CMAKE project
- hit the build button
- done

### Hardest parts of this project in most to least  order
- Adding Some Dependencies to CMAKE
- Understanding Vulkan Descriptors
- Initializing Vulkan
- Loading GLTF
- Adding Imgui
- Using Vulkan Mem Allocator
