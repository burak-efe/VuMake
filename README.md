# Terrible Vulkan Renderer

This is a terrible vulkan renderer, barely functioning.
It's a CMAKE project with C++ 23 required, because I used println twice 💀

### It Uses:
- Vulkan Memory Allocator
- IMGUI
- FastGLTF
- GLFW
- GLM

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
