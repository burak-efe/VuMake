# VuMake: A Terrible Vulkan Renderer
![2025-03-22 08_40_35-Window](https://github.com/user-attachments/assets/afd97f4a-7664-454d-a0c2-002d9e2f95af)

This is a terrible vulkan renderer, barely functioning.
This renderer is meant to be prototyping space for my purposes and I would not recommend to use on any project.

Some non-optimal things in this project, that should be avioded in normal circumstances:
- Only header files: to faster prorotyping
- A lot of third party library usage: Im just experimenting with them
- Excessive file count: because I like it
- Global, static variables global context: to make prototyping faster
- Unnecesary ECS: im just experimenting

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
- flecs
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
