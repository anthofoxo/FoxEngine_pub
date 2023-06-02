# FoxEngine

FoxEngine is a small game engine created in C and C++, featuring lua as a scripting language.

This project is very early on and major changes **will** occur.

## The FoxEngine team
* [AnthoFoxo](https://anthofoxo.xyz) - Lead developer, Project director
* [Simon Bolivar](https://github.com/SNSTRUTHERS) - Code review, API design

## Dependencies
All dependencies are included in the source tree or as submodules (Make sure to clone with submodules! `--recurse-submodules`)
* [glfw 3.3.8](https://github.com/glfw/glfw/tree/3.3.8)
* [glm 0.9.9.8](https://github.com/g-truc/glm/tree/0.9.9.8)
* [assimp v5.0.1](https://github.com/assimp/assimp/tree/v5.0.1)
* [entt v3.11.1](https://github.com/skypjack/entt/tree/v3.11.1)
* [spdlog v1.11.0](https://github.com/gabime/spdlog/tree/v1.11.0)
* [imgui f0f3b92](https://github.com/ocornut/imgui/blob/f0f3b927e0a36ccf06c7a6d29e586f49365f50d7)
* [stb_image.h 5736b15](https://github.com/nothings/stb/blob/5736b15f7ea0ffb08dd38af21067c314d6a3aae9/stb_image.h)
* [debug-trap.h 84abba9] (https://github.com/nemequ/portable-snippets/blob/84abba93ff3d52c87e08ba81de1cc6615a42b72e/debug-trap/debug-trap.h)
* [OpenGL Core 3.3+](https://gen.glad.sh/#generator=c&api=gl%3D3.3&profile=gl%3Dcore%2Cgles1%3Dcommon&extensions=GL_KHR_debug)
* Optional OpenGL Extensions:
* * [KHR_debug](https://registry.khronos.org/OpenGL/extensions/KHR/KHR_debug.txt)
(Partial implmentation)

## Building

premake5 is used for creating project scripts.
https://premake.github.io/

* `premake5 vs2022` For visual studio
* `premake5 gmake2` For make files